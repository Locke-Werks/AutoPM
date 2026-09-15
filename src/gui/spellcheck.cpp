#include "spellcheck.h"

#include "theme.h"

#include <QSet>
#include <QTextCharFormat>

#ifdef _WIN32
#include <windows.h>
#include <spellcheck.h>
#include <wrl/client.h>
#endif

namespace spell {
namespace {

#ifdef _WIN32
Microsoft::WRL::ComPtr<ISpellChecker> g_checker;
bool g_comReady = false;
#endif
QSet<QString> g_ignored;

} // namespace

void initialise() {
#ifdef _WIN32
    // The MCP front end never calls this, so COM is only started where there
    // is a window to use it.
    const HRESULT started = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    g_comReady = SUCCEEDED(started);
    if (!g_comReady && started != RPC_E_CHANGED_MODE) return;

    Microsoft::WRL::ComPtr<ISpellCheckerFactory> factory;
    if (FAILED(CoCreateInstance(__uuidof(SpellCheckerFactory), nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&factory))))
        return;

    // The user's own display language, so the tool agrees with the rest of
    // their machine about what a word is.
    wchar_t tag[LOCALE_NAME_MAX_LENGTH] = {};
    if (GetUserDefaultLocaleName(tag, LOCALE_NAME_MAX_LENGTH) == 0) wcscpy_s(tag, L"en-US");

    BOOL supported = FALSE;
    if (FAILED(factory->IsSupported(tag, &supported)) || !supported) {
        if (FAILED(factory->IsSupported(L"en-US", &supported)) || !supported) return;
        wcscpy_s(tag, L"en-US");
    }
    factory->CreateSpellChecker(tag, &g_checker);
#endif
}

void shutdown() {
#ifdef _WIN32
    g_checker.Reset();
    if (g_comReady) CoUninitialize();
    g_comReady = false;
#endif
}

bool available() {
#ifdef _WIN32
    return g_checker != nullptr;
#else
    return false;
#endif
}

QVector<Span> mistakesIn(const QString& text) {
    QVector<Span> found;
#ifdef _WIN32
    if (!g_checker || text.isEmpty()) return found;

    Microsoft::WRL::ComPtr<IEnumSpellingError> errors;
    if (FAILED(g_checker->Check(reinterpret_cast<LPCWSTR>(text.utf16()), &errors))) return found;

    Microsoft::WRL::ComPtr<ISpellingError> error;
    while (errors->Next(&error) == S_OK && error) {
        ULONG start = 0, length = 0;
        error->get_StartIndex(&start);
        error->get_Length(&length);
        error.Reset();
        if (length == 0) continue;

        // A word the user has told this session to leave alone is not a
        // mistake, even though the operating system still thinks it is.
        const QString word = text.mid(static_cast<int>(start), static_cast<int>(length));
        if (g_ignored.contains(word)) continue;
        found.push_back({static_cast<int>(start), static_cast<int>(length)});
    }
#else
    Q_UNUSED(text);
#endif
    return found;
}

QStringList suggestionsFor(const QString& word) {
    QStringList out;
#ifdef _WIN32
    if (!g_checker || word.isEmpty()) return out;
    Microsoft::WRL::ComPtr<IEnumString> options;
    if (FAILED(g_checker->Suggest(reinterpret_cast<LPCWSTR>(word.utf16()), &options))) return out;

    LPOLESTR text = nullptr;
    while (options->Next(1, &text, nullptr) == S_OK && text) {
        out << QString::fromWCharArray(text);
        CoTaskMemFree(text);
        text = nullptr;
        if (out.size() >= 8) break;
    }
#else
    Q_UNUSED(word);
#endif
    return out;
}

void ignoreWord(const QString& word) {
    g_ignored.insert(word);
#ifdef _WIN32
    if (g_checker) g_checker->Ignore(reinterpret_cast<LPCWSTR>(word.utf16()));
#endif
}

void addToDictionary(const QString& word) {
    g_ignored.insert(word);
#ifdef _WIN32
    if (g_checker) g_checker->Add(reinterpret_cast<LPCWSTR>(word.utf16()));
#endif
}

} // namespace spell

SpellHighlighter::SpellHighlighter(QTextDocument* document) : QSyntaxHighlighter(document) {}

void SpellHighlighter::highlightBlock(const QString& text) {
    if (!spell::available()) return;

    QTextCharFormat wrong;
    wrong.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    // The house has no red, so the squiggle borrows the colour that already
    // means "this is wrong" everywhere else in the app.
    wrong.setUnderlineColor(theme::danger());

    for (const spell::Span& span : spell::mistakesIn(text))
        setFormat(span.start, span.length, wrong);
}

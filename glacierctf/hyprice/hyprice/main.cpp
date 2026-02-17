#include <iostream> // for std::cin
#include <print>
#include <vector>
#include <poll.h>

// not relevant, just some base64
#include "base64.hpp"

// hyprutils 0.10.2
#include <hyprutils/animation/AnimatedVariable.hpp>
#include <hyprutils/animation/AnimationConfig.hpp>
#include <hyprutils/animation/AnimationManager.hpp>

#include <hyprutils/memory/SharedPtr.hpp>
#include <hyprutils/memory/UniquePtr.hpp>
#include <hyprutils/memory/WeakPtr.hpp>
#include <hyprutils/string/VarList2.hpp>

using namespace Hyprutils::Memory;

#define SP CSharedPointer
#define WP CWeakPointer
#define UP CUniquePointer

class EmptyContext {};
class CMyAnimationManager;
using CAnimatedInt = Hyprutils::Animation::CGenericAnimatedVariable<size_t, EmptyContext>;
using ANIMINT      = SP<CAnimatedInt>;

// Ignore all the ansi stuff. It's just for the terminal appearance.
#define CSI             "\033["
#define OSC             "\033]"
#define RESET           CSI "m"
#define UNDERLINE       CSI "4m"
#define NOUNDERLINE     CSI "24m"
#define ERASEENTIRE     CSI "2J"
#define ERASEENTIRELINE CSI "2K"
#define SAVECURSOR      CSI "s"
#define RESTORECURSOR   CSI "u"
#define ROWUP(n)        (n == 0 ? "" : std::format(CSI "{}A", n))
#define ROWDOWN(n)      (n == 0 ? "" : std::format(CSI "{}B", n))
#define ROWINSERT(n)    (n == 0 ? "" : std::format(CSI "{}L", n))
#define ROWDELETE(n)    (n == 0 ? "" : std::format(CSI "{}M", n))
#define COLPOS(col)     std::format(CSI "{}G", col)

#define PATHMAX 0x100
#define NAMEMAX 0x100

template <size_t N>
class CFixed {
    std::array<char, N> m_data = {0};
    size_t              m_size = 0;

  public:
    CFixed() : m_size(0) {};
    CFixed(std::string_view inital) {
        put(inital);
    };
    void put(std::string_view content) {
        m_size = content.copy(m_data.data(), m_data.size());
    };
    std::string_view view() {
        return std::string_view(m_data.data(), m_size);
    };
};

struct CDotfile {
  public:
    CDotfile(std::string_view path, std::string_view content);
    void edit(std::string_view content);
    void score();

    std::string     m_content = "";
    int64_t         m_score   = 0;
    CFixed<PATHMAX> m_path;
};

struct CRice {
  public:
    CRice(std::string_view name);
    ~CRice() = default;

    CFixed<NAMEMAX>           m_name;

    uint64_t                  m_version = 0;
    WP<CRice>                 m_forkedFrom;

    std::vector<SP<CDotfile>> m_dots;
    void                      updateScore();
    void                      dotEdit(std::string_view path, std::string_view content);
    void                      dotYank(std::stringstream& frame, std::string_view path);
    void                      fork(std::string_view name);

    //
    ANIMINT m_score = nullptr;
    void    render(std::stringstream& out);
};

class CHyprice {
  public:
    CHyprice();
    ~CHyprice() = default;

    void prompt(std::string_view promptText, std::function<void(std::string_view)> handler);
    void handleInput();
    void handleMain(std::string_view input);

    void loop();

    struct {
        UP<CMyAnimationManager>                    manager;
        Hyprutils::Animation::CAnimationConfigTree tree;
    } m_animation;

    struct {
        std::function<void(std::string_view)> handler;
        bool waiting = false;
        bool base64  = false;
    } m_inputState;

    std::vector<UP<CRice>> m_rices;
    std::stringstream      m_frame;
    CFixed<8>              m_trophyChar;
};

inline UP<CHyprice> g_hyprice;

class CMyAnimationManager : public Hyprutils::Animation::CAnimationManager {
  public:
    void         tick();
    void         createAnimation(uint64_t initial, SP<CAnimatedInt>& av);
    virtual void scheduleTick() {}
    virtual void onTicked() {}
};

static std::optional<size_t> convertNumber(std::string_view text) {
    size_t     number = 0;
    const auto RESULT = std::from_chars(text.data(), text.data() + text.size(), number);
    if (RESULT.ec == std::errc())
        return number;
    else
        return {};
}

CDotfile::CDotfile(std::string_view path, std::string_view content) {
    m_path.put(path);
    edit(content);
}

void CDotfile::edit(std::string_view content) {
    m_content.clear();
    std::ranges::copy(content.begin(), content.end(), std::back_inserter(m_content));
    score();
}

void CDotfile::score() {
    if (m_content.size() > 0x1000)
        return;

    // bias
    int64_t res = 10;

    // placeholder for llm slop
    const std::array<std::string_view, 6>  BADTOKENS  = {"unsafe", "C", "C++", "zig", "wayland", "pointer"};
    const std::array<std::string_view, 10> GOODTOKENS = {"safe", "rust", "hypr", "nix", "LosFuzzys", "Glacier", "CTF", "pwn", "X11", "reference"};
    std::istringstream                     lines(m_content);
    std::string                            line;
    while (std::getline(lines, line)) {
        auto BAD  = std::ranges::count_if(BADTOKENS, [&line](std::string_view t) { return line.contains(t); });
        auto GOOD = std::ranges::count_if(GOODTOKENS, [&line](std::string_view t) { return line.contains(t); });
        res -= BAD;
        res += GOOD;
    }

    m_score = res;
}

CRice::CRice(std::string_view name) : m_version(0), m_name(std::format("🧊 {}", name)) {
    g_hyprice->m_animation.manager->createAnimation(0, m_score);
};

void CRice::updateScore() {
    const auto NEWSCORE = std::ranges::fold_left(m_dots, 0ll, [](int64_t res, const SP<CDotfile>& dot) { return res + dot->m_score; });
    *m_score            = NEWSCORE;
    // Grind away and one day you will get a working system (or a trophy)
    if (m_score->goal() > (1lu << 32))
        m_score->setCallbackOnEnd([this](auto ref) { m_name.put(g_hyprice->m_trophyChar.view()); });
}

void CRice::dotEdit(std::string_view path, std::string_view content) {
    if (path.size() > PATHMAX)
        return;

    m_version += 1;
    const auto DOTIT = std::ranges::find_if(m_dots, [&path](SP<CDotfile>& x) { return std::ranges::equal(x->m_path.view(), path); });
    if (DOTIT != m_dots.end() && (*DOTIT).strongRef() < 2) {
        (*DOTIT)->edit(content);
        return;
    }

    if (DOTIT != m_dots.end())
        m_dots.erase(DOTIT);

    m_dots.emplace_back(makeShared<CDotfile>(path, content));
}

void CRice::dotYank(std::stringstream& frame, std::string_view path) {
    const auto DOTIT = std::ranges::find_if(m_dots, [&path](SP<CDotfile>& x) { return std::ranges::equal(path, x->m_path.view()); });
    if (DOTIT == m_dots.end())
        return;

    frame << OSC "52;c;" << base64::to_base64((*DOTIT)->m_content) << "\x07";
}

// NOTE: this is latest git beta early access
void CRice::fork(std::string_view name) {
    auto res     = makeUnique<CRice>(name);
    res->m_score = m_score;
    res->m_dots  = std::vector<SP<CDotfile>>(m_dots.begin(), m_dots.end());

    g_hyprice->m_rices.emplace_back(std::move(res));
}

void CRice::render(std::stringstream& frame) {
    frame << std::format("{}-", m_name.view());
    frame << ((m_version << 0x10) & 0x10) << "." << ((m_version << 8) & 8) << "." << (m_version & 0xff);
    frame << " Score: " << m_score->value();
    if (m_score->isBeingAnimated()) {
        if (m_score->value() < m_score->goal())
            frame << " ↑";
        else
            frame << " ↓";
    }
}

CHyprice::CHyprice() : m_trophyChar("⭐") {
    m_animation.manager = makeUnique<CMyAnimationManager>();
    m_animation.manager->addBezierWithName("slope", {0., 0.5}, {0.5, 0.5});
    m_animation.tree.createNode("score");
    m_animation.tree.setConfigForNode("score", 1, 25.0, "slope");
}

void CHyprice::prompt(std::string_view promptText, std::function<void(std::string_view)> handler) {
    if (m_inputState.waiting)
        return;

    m_frame << "\n" << (promptText.empty() ? "│️" : "├─️") << promptText << "\n" ERASEENTIRELINE "╰️→ " UNDERLINE SAVECURSOR;
    m_inputState.handler = handler;
    m_inputState.waiting = true;
}

void CHyprice::handleInput() {
    if (!m_inputState.waiting)
        return;

    std::string line;
    if (!std::getline(std::cin, line)) {
        std::println(stderr, "IO Broke!");
        exit(1);
    }

    m_inputState.waiting = false;

    m_frame << NOUNDERLINE ERASEENTIRELINE "\r├→ ";
    m_frame << UNDERLINE << (line.empty() ? "×" : line);
    m_frame << NOUNDERLINE;

    if (m_inputState.base64) {
        line                = base64::from_base64(line);
        m_inputState.base64 = false;
    }

    m_inputState.handler(line);

    if (!m_inputState.waiting)
        m_frame << ROWDOWN(1) << "\r";
}

void CHyprice::handleMain(std::string_view answer) {
    Hyprutils::String::CVarList2 args(std::string(answer), 2, ' ', true);
    if (args.size() < 2)
        return;

    if (args[0] == "trophy" && args.size() == 2) {
        m_trophyChar.put(args[1]);
        return;
    }

    const auto RICEIT = std::ranges::find_if(m_rices, [name = args[1]](UP<CRice>& x) { return x->m_name.view().ends_with(name); });
    if (args[0] == "new" && args.size() == 2 && RICEIT == m_rices.end()) {
        m_rices.emplace_back(makeUnique<CRice>(args[1]));
        return;
    }
    if (RICEIT == m_rices.end())
        return;

    if (args[0] == "rm" && args.size() == 2)
        m_rices.erase(RICEIT);
    else if (args[0] == "edit" && args.size() >= 3) {
        WP<CRice>         WEAKRICE = *RICEIT;
        const std::string PATH{args[2]}; // copy for capture
        m_inputState.base64 = args.size() == 4 && args[3] == "b64";
        prompt(m_inputState.base64 ? "Enter content in base64" : "Enter content", [WEAKRICE, PATH](std::string_view answer) {
            if (WEAKRICE) {
                WEAKRICE->dotEdit(PATH, answer);
                WEAKRICE->updateScore();
            }
        });
    } else if (args[0] == "yank" && args.size() == 3) {
        (*RICEIT)->dotYank(m_frame, args[2]);
    } else if (args[0] == "fork" && args.size() == 3) {
        const auto FORKNAMEIT = std::ranges::find_if(m_rices, [name = args[2]](UP<CRice>& x) { return name == x->m_name.view(); });
        if (FORKNAMEIT != m_rices.end())
            return;

        (*RICEIT)->fork(args[2]);
    }
}

// If it is not obvious to you why this simple terminal application has an eventloop,
// it's because it can update the text while waiting on your input, but also doesn't busy wait.
void CHyprice::loop() {
    const auto LINESMAX    = 10lu;
    const auto SEPERATOR   = 1;
    size_t     headerLines = 0;
    size_t     logLines    = 0;

    std::println("");

    pollfd input[1] = {{fd: 0, events: POLLIN}};
    while (true) {
        const bool NEEDSTICK = m_animation.manager->shouldTickForNext();
        if (m_inputState.waiting) {
            int events = poll(input, 1, NEEDSTICK ? /* frame throttle */ 100 : /* wait for input */ 10000);

            if (input[0].revents & POLLERR)
                break;

            if (input[0].revents & POLLHUP)
                break;

            if (events < 0) {
                if (errno == EINTR)
                    continue;
                else
                    break;
            }
        } else
            input[0].revents = 0;
        const auto INPUTAVAILABLE = input[0].revents & POLLIN;

        if (NEEDSTICK)
            m_animation.manager->tick();

        std::stringstream previousFrame{};
        m_frame.swap(previousFrame);
        m_frame << RESET;

        if (m_inputState.waiting && !INPUTAVAILABLE)
            m_frame << SAVECURSOR << ROWUP(headerLines + SEPERATOR + logLines - 1);
        else
            m_frame << ROWUP(headerLines + SEPERATOR + logLines);

        if (headerLines < m_rices.size())
            m_frame << ROWDOWN(headerLines) << ROWINSERT(m_rices.size() - headerLines) << ROWUP(headerLines);
        else if (headerLines > m_rices.size())
            m_frame << ROWDELETE(headerLines - m_rices.size());

        for (auto& rice : m_rices) {
            m_frame << ERASEENTIRELINE "\r│️ ";
            rice->render(m_frame);
            m_frame << ROWDOWN(1);
        }

        headerLines = m_rices.size();

        m_frame << ERASEENTIRELINE "\r├─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️─️";
        m_frame << ROWDOWN(logLines);

        if (!m_inputState.waiting)
            prompt("Provide a command", [this](std::string_view answer) { handleMain(answer); });
        else if (INPUTAVAILABLE) {
            handleInput();
        } else
            m_frame << RESTORECURSOR;

        logLines += std::ranges::count(m_frame.str(), '\n');
        if (logLines > LINESMAX) {
            m_frame << ROWUP(logLines - 1) << ROWDELETE(logLines - LINESMAX) << ROWDOWN(LINESMAX - 1);
            if (m_inputState.waiting)
                m_frame << COLPOS(4) << SAVECURSOR;

            logLines = LINESMAX;
        }

        std::print("{}", m_frame.view());
    }
}

int main() {
    std::setbuf(stdout, nullptr);
    //std::setbuf(stdin, nullptr);

    g_hyprice = makeUnique<CHyprice>();
    g_hyprice->loop();
}

void CMyAnimationManager::tick() {
    for (size_t i = 0; i < m_vActiveAnimatedVariables.size(); i++) {
        const auto PAV = m_vActiveAnimatedVariables[i].lock();
        if (!PAV || !PAV->ok() || !PAV->isBeingAnimated())
            continue;

        const auto SPENT   = PAV->getPercent();
        const auto PBEZIER = getBezier(PAV->getBezierName());

        if (SPENT >= 1.f || !PAV->enabled()) {
            // calls end callback
            PAV->warp(true, false);
            continue;
        }

        const auto POINTY = PBEZIER->getYForPoint(SPENT);
        auto       avInt  = dc<CAnimatedInt*>(PAV.get());

        const auto DELTA = avInt->goal() - avInt->begun();
        avInt->value()   = avInt->begun() + (DELTA * POINTY);

        PAV->onUpdate();
    }

    tickDone();
}

void CMyAnimationManager::createAnimation(uint64_t initial, SP<CAnimatedInt>& av) {
    const auto PAV = makeShared<CAnimatedInt>();

    PAV->create(0, sc<Hyprutils::Animation::CAnimationManager*>(this), PAV, initial);
    PAV->setConfig(g_hyprice->m_animation.tree.getConfig("score"));
    av = std::move(PAV);
}

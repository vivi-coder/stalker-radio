#include <gtk/gtk.h>
#include <glib.h>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

// ─────────────────────────────────────────────
//  Data types
// ─────────────────────────────────────────────

enum class MsgType { CHATTER, WARNING, DANGER, SYSTEM, AIR, AIR_DANGER };

struct Squad {
    std::string id, name, faction, freq, color;
    int members;
    bool isViper;
    std::string viperRole, viperStatus;
    int alt, spd;
};

struct Message {
    std::string sender, text, freq, time, squadName, color;
    MsgType type;
    bool isAir;
    std::string viperRole, viperStatus;
};

// ─────────────────────────────────────────────
//  App state
// ─────────────────────────────────────────────

struct AppState {
    std::vector<Squad>   squads;
    std::vector<Squad>   vipers;
    std::vector<Message> messages;
    std::string          activeFilter = "all";
    bool                 paused       = false;
    int                  txCount      = 0;
    int                  anomalyCount = 0;
    time_t               startTime;
    int                  signalLevel  = 4;

    GtkWidget *feed         = nullptr;
    GtkWidget *freqLabel    = nullptr;
    GtkWidget *altLabel     = nullptr;
    GtkWidget *spdLabel     = nullptr;
    GtkWidget *altRow       = nullptr;
    GtkWidget *clockLabel   = nullptr;
    GtkWidget *txLabel      = nullptr;
    GtkWidget *msgCount     = nullptr;
    GtkWidget *anomalyLabel = nullptr;
    GtkWidget *uptimeLabel  = nullptr;
    GtkWidget *pauseBtn     = nullptr;
    GtkWidget *signalBox    = nullptr;
    GtkWidget *activityBox  = nullptr;

    std::mt19937 rng;
    AppState() : rng(std::random_device{}()) { startTime = time(nullptr); }
};

static AppState *app = nullptr;

// ─────────────────────────────────────────────
//  String pools
// ─────────────────────────────────────────────

static const std::vector<std::string> NAMES_LONER   = {"Ghost","Marked One","Nimble","Fox","Strelok","Wolf","Beard","Fang","Skif","Tracker","Razor","Seriy"};
static const std::vector<std::string> NAMES_DUTY    = {"Voronin","Petrenko","Shulga","Tur","Ivanchuk","Major","Sergeant","Colonel"};
static const std::vector<std::string> NAMES_FREEDOM = {"Lukash","Chekhov","Max","Yar","Yoga","Bes","Karpov"};
static const std::vector<std::string> NAMES_MERCS   = {"Jackal","Drifter","Kane","Morgan","Reaper","Shade"};
static const std::vector<std::string> NAMES_BANDIT  = {"Mitya","Kolya","Brodyaga","Gruzin","Scrapper","Teeth"};
static const std::vector<std::string> NAMES_ECO     = {"Prof","Sakharov","Doc","Analyst","Vega"};

static const std::vector<std::string> CHATTER = {
    "Sector clear. Moving to next waypoint.",
    "Anyone got bread? Haven't eaten since morning.",
    "Watch the left flank. Saw movement.",
    "Zone's been quiet. Too quiet.",
    "Bandits spotted near the bridge. Avoid that route.",
    "Good haul from the anomaly field. Worth the risk.",
    "Running low on ammo. Need a resupply drop.",
    "Heard a stalker found an artifact near the factory.",
    "Don't trust the guides here. Half work for mercs.",
    "That emission knocked out half our gear.",
    "Camp fire's lit. Warm food while it lasts.",
    "My suit's busted. Anyone carrying repair kits?",
    "Stay away from the marshes at night.",
    "Copy that. Maintaining position.",
    "Found a dead stalker near the viaduct. PDA intact.",
    "Something's wrong with the compass. Zone's at it.",
    "We're down to three. Need extraction.",
    "Artifact detected. Triangulating position.",
    "Signal lost between the pylons again.",
    "Meet at the checkpoint in two hours.",
    "Keep chatter minimum. Something's out there.",
    "Brother, the Zone takes everyone in the end.",
    "Gravitational anomaly blocked our path. Long way round.",
    "Anyone seen Nimble? He was supposed to check in.",
    "Bloodsucker near the warehouse. Do not approach.",
    "Snork pack moving south. Three of them at least.",
    "Wind's picking up. Psy-storm incoming.",
    "PDA battery dying. Switching to backup.",
    "We buried Petrov near the old barn. Mark your maps.",
    "Emission residue still high. Suits on.",
};

static const std::vector<std::string> WARNINGS = {
    "CONTACT! Multiple targets, northeast!",
    "Anomaly field expanding! Pull back!",
    "PSY-STORM WARNING - 10 minutes to shelter!",
    "We're taking fire! Requesting backup!",
    "Stalker down! We need a medic now!",
    "Blowout detected on Zone sensors. FIND COVER.",
    "LOST COMMS with Alpha team. Last position - the Cordon.",
    "Mutant herd moving fast. Don't get caught open.",
    "Bandit ambush at checkpoint 7! Abort your route!",
    "Gravity anomaly surge near the antenna. Stay back.",
};

static const std::vector<std::string> DANGERS = {
    "MAN DOWN. REPEAT, MAN DOWN.",
    "WE ARE OVERRUN. FALL BACK TO THE PERIMETER.",
    "BLOWOUT IMMINENT - ALL STALKERS SEEK COVER NOW.",
    "HOSTILE SQUAD ON OUR FREQUENCY. ABORT.",
    "FIREBASE COMPROMISED. EVACUATE IMMEDIATELY.",
    "CONTROLLER SIGHTED - DO NOT ENGAGE. RUN.",
    "EMISSION WAVE - TAKING CRITICAL DAMAGE.",
};

static const std::vector<std::string> SYSTEMS = {
    "[PDA] Transmission degraded. Signal interference detected.",
    "[SCANNER] New squad beacon detected on this frequency.",
    "[NET] Encryption handshake failed. Channel exposed.",
    "[ZONE] Anomaly activity: elevated.",
    "[RELAY] Signal bounced through dead zone. Source unknown.",
    "[PDA] Battery at 12%. Comms may drop.",
    "[NET] Broadcast from unregistered PDA intercepted.",
    "[ZONE] Geiger spike at Sector 7. Unconfirmed.",
};

static const std::vector<std::string> AIR_CHATTER = {
    "Altitude check. Holding steady, rotor nominal.",
    "Zone thermals are rough today. Watch your instruments.",
    "Visual on the Cordon. No unusual movement.",
    "Cloud ceiling dropping. Switching to instruments.",
    "Marking waypoint Delta. ETA two minutes.",
    "Ground team, we're orbiting your position. Say status.",
    "Fuel at sixty percent. Continuing mission.",
    "Anomaly cluster visible from altitude. Logging coords.",
    "That emission pulse messed with GPS. Flying manual.",
    "Thermal bloom at grid seven. Not natural.",
    "Visibility poor near the plant. Proceeding with caution.",
    "Ground contact, we have eyes on your position. Looks clear.",
    "Scanning the ravine. No contacts.",
    "Rotor wash might stir the anomaly field. Banking left.",
    "Picking up interference on nav band. Zone's playing games.",
    "Cleared to descend. Watching for those pylon cables.",
    "Steady at altitude. Nothing on thermals.",
    "Perimeter sweep complete. No contacts on this pass.",
};

static const std::vector<std::string> AIR_WARNINGS = {
    "TAKING GROUND FIRE. BREAKING OFF.",
    "ENGINE STUTTER AT ALTITUDE - ASSESSING.",
    "ANOMALY DISCHARGE - LOST MAIN ROTOR CONTROL BRIEFLY.",
    "UNIDENTIFIED AIRCRAFT ON INTERCEPT COURSE.",
    "TERRAIN PROXIMITY - PULLING UP.",
    "BLOWOUT SHOCKWAVE AT ALTITUDE. ASSESSING DAMAGE.",
    "NAVIGATION FAILURE. FLYING ON COMPASS ONLY.",
};

static const std::vector<std::string> AIR_COMBAT = {
    "ENGAGING GROUND TARGETS. ROCKETS AWAY.",
    "SUPPRESSING BANDIT POSITION. GROUND TEAM, STAY LOW.",
    "GUNS HOT. ON ATTACK RUN.",
    "FIRST PASS COMPLETE. RE-ENGAGING.",
    "VIPER FLIGHT - BREAK LEFT. AAA ACTIVE.",
    "TARGET NEUTRALIZED. RETURNING TO OVERWATCH.",
    "ALL VIPERS FORM ON ME. ANOTHER PASS.",
    "HEAVY DAMAGE. BREAKING OFF AND HEADING HOME.",
};

static const std::vector<std::string> NOISE = {
    "...static...", "...ksssshk...", "......",
    "...krrk...signal lost...", "...bzzzt...",
};

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────

template<typename T>
static const T& pick(const std::vector<T>& v, std::mt19937& rng) {
    std::uniform_int_distribution<size_t> d(0, v.size() - 1);
    return v[d(rng)];
}

static std::string pickName(const std::string& faction, std::mt19937& rng) {
    if (faction == "duty")      return pick(NAMES_DUTY,    rng);
    if (faction == "freedom")   return pick(NAMES_FREEDOM, rng);
    if (faction == "mercs")     return pick(NAMES_MERCS,   rng);
    if (faction == "bandit")    return pick(NAMES_BANDIT,  rng);
    if (faction == "ecologist") return pick(NAMES_ECO,     rng);
    return pick(NAMES_LONER, rng);
}

static std::string currentTime() {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << t->tm_hour << ":"
       << std::setw(2) << std::setfill('0') << t->tm_min  << ":"
       << std::setw(2) << std::setfill('0') << t->tm_sec;
    return ss.str();
}

static std::string uptimeStr() {
    time_t up = time(nullptr) - app->startTime;
    if (up < 60) return std::to_string(up) + "s";
    return std::to_string(up / 60) + "m " + std::to_string(up % 60) + "s";
}

static std::string cssColor(MsgType t) {
    switch (t) {
        case MsgType::DANGER:     return "#ff5555";
        case MsgType::WARNING:    return "#ffaa00";
        case MsgType::SYSTEM:     return "#2a7a4a";
        case MsgType::AIR:        return "#aaeeff";
        case MsgType::AIR_DANGER: return "#ff8800";
        default:                  return "#88cc99";
    }
}

static std::string borderColor(MsgType t) {
    switch (t) {
        case MsgType::DANGER:     return "#ff5555";
        case MsgType::WARNING:    return "#ffaa00";
        case MsgType::SYSTEM:     return "#2a7a4a";
        case MsgType::AIR:        return "#00cfff";
        case MsgType::AIR_DANGER: return "#ff8800";
        default:                  return "#2a4a2a";
    }
}

// Attach a one-shot CSS provider to a single widget.
// This is the same approach used throughout GTK4 for per-widget styling;
// gtk_style_context_add_provider is still the correct API at priority
// APPLICATION (the deprecation warning is a GTK4.10 quirk for the
// global-display variant, not the per-widget variant, but we suppress
// it with -Wno-deprecated-declarations in CMakeLists).
static void applyCSS(GtkWidget *w, const std::string& css) {
    GtkCssProvider *p = gtk_css_provider_new();
    gtk_css_provider_load_from_string(p, css.c_str());
    gtk_style_context_add_provider(
        gtk_widget_get_style_context(w),
        GTK_STYLE_PROVIDER(p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(p);
}

// ─────────────────────────────────────────────
//  Message generation
// ─────────────────────────────────────────────

static Message generateMessage() {
    std::uniform_real_distribution<double> d(0.0, 1.0);
    double r = d(app->rng);

    if (r < 0.22) {
        std::vector<Squad*> airborne;
        for (auto& v : app->vipers)
            if (v.viperStatus == "AIRBORNE") airborne.push_back(&v);

        if (!airborne.empty()) {
            std::uniform_int_distribution<size_t> idx(0, airborne.size() - 1);
            Squad* viper = airborne[idx(app->rng)];

            double r2 = d(app->rng);
            MsgType type;
            std::string text;
            if (r2 < 0.08) {
                type = MsgType::AIR_DANGER;
                text = pick(AIR_COMBAT, app->rng);
                app->anomalyCount++;
            } else if (r2 < 0.22) {
                type = MsgType::AIR_DANGER;
                text = pick(AIR_WARNINGS, app->rng);
            } else {
                type = MsgType::AIR;
                text = pick(AIR_CHATTER, app->rng);
            }
            return { viper->name, text, viper->freq, currentTime(),
                     viper->name, "#00cfff",
                     type, true, viper->viperRole, viper->viperStatus };
        }
    }

    std::uniform_int_distribution<size_t> si(0, app->squads.size() - 1);
    Squad& squad = app->squads[si(app->rng)];

    std::string sender;
    MsgType type;
    std::string text;

    if (r < 0.04) {
        type   = MsgType::DANGER;
        text   = pick(DANGERS, app->rng);
        sender = pickName(squad.faction, app->rng);
        app->anomalyCount++;
    } else if (r < 0.14) {
        type   = MsgType::WARNING;
        text   = pick(WARNINGS, app->rng);
        sender = pickName(squad.faction, app->rng);
    } else if (r < 0.21) {
        type   = MsgType::SYSTEM;
        text   = pick(SYSTEMS, app->rng);
        sender = "SYSTEM";
    } else {
        type   = MsgType::CHATTER;
        text   = pick(CHATTER, app->rng);
        sender = pickName(squad.faction, app->rng);
    }

    return { sender, text, squad.freq, currentTime(),
             squad.name, squad.color, type, false, "", "" };
}

// ─────────────────────────────────────────────
//  Feed rendering
// ─────────────────────────────────────────────

static bool msgMatchesFilter(const Message& m) {
    if (app->activeFilter == "all")     return true;
    if (app->activeFilter == "air")     return m.isAir;
    if (app->activeFilter == "danger")  return m.type == MsgType::DANGER
                                            || m.type == MsgType::AIR_DANGER
                                            || m.type == MsgType::WARNING;
    if (app->activeFilter == "chatter") return m.type == MsgType::CHATTER;
    return true;
}

static GtkWidget* buildMessageRow(const Message& m) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_margin_start(box, 4);
    gtk_widget_set_margin_end(box, 4);
    gtk_widget_set_margin_top(box, 3);
    gtk_widget_set_margin_bottom(box, 3);
    applyCSS(box, "box { border-left:2px solid " + borderColor(m.type) + "; padding-left:6px; }");

    GtkWidget *meta = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    GtkWidget *timeLabel = gtk_label_new(m.time.c_str());
    gtk_widget_add_css_class(timeLabel, "msg-time");
    gtk_box_append(GTK_BOX(meta), timeLabel);

    std::string senderText = m.isAir ? ("[AIR] " + m.sender) : m.sender;
    GtkWidget *senderLabel = gtk_label_new(senderText.c_str());
    applyCSS(senderLabel, "label { color:" + m.color + "; font-weight:bold; font-size:11px; }");
    gtk_box_append(GTK_BOX(meta), senderLabel);

    std::string freqStr;
    if (m.isAir)
        freqStr = "[" + m.freq + " MHz / " + m.viperRole + " / " + m.viperStatus + "]";
    else if (m.type != MsgType::SYSTEM)
        freqStr = "[" + m.freq + " MHz / " + m.squadName + "]";

    if (!freqStr.empty()) {
        GtkWidget *fl = gtk_label_new(freqStr.c_str());
        gtk_widget_add_css_class(fl, "msg-freq");
        gtk_box_append(GTK_BOX(meta), fl);
    }

    gtk_box_append(GTK_BOX(box), meta);

    GtkWidget *textLabel = gtk_label_new(m.text.c_str());
    gtk_label_set_wrap(GTK_LABEL(textLabel), TRUE);
    gtk_label_set_xalign(GTK_LABEL(textLabel), 0.0f);
    applyCSS(textLabel, "label { color:" + cssColor(m.type) + "; font-size:12px; }");
    gtk_box_append(GTK_BOX(box), textLabel);

    return box;
}

static void scrollFeedToBottom() {
    GtkWidget *sw = gtk_widget_get_parent(gtk_widget_get_parent(app->feed));
    if (GTK_IS_SCROLLED_WINDOW(sw)) {
        GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(sw));
        gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));
    }
}

static void rebuildFeed() {
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(app->feed)) != nullptr)
        gtk_box_remove(GTK_BOX(app->feed), child);
    for (const auto& m : app->messages) {
        if (!msgMatchesFilter(m)) continue;
        gtk_box_append(GTK_BOX(app->feed), buildMessageRow(m));
    }
    g_idle_add_once([](gpointer) { scrollFeedToBottom(); }, nullptr);
}

static void appendMessageToFeed(const Message& m) {
    if (!msgMatchesFilter(m)) return;
    gtk_box_append(GTK_BOX(app->feed), buildMessageRow(m));
    g_idle_add_once([](gpointer) { scrollFeedToBottom(); }, nullptr);
}

// ─────────────────────────────────────────────
//  Signal / activity bars
// ─────────────────────────────────────────────

static void updateSignalBars(bool isViper) {
    std::uniform_int_distribution<int> d(-1, 1);
    app->signalLevel = std::max(1, std::min(5, app->signalLevel + d(app->rng)));
    GtkWidget *c = gtk_widget_get_first_child(app->signalBox);
    int i = 0;
    while (c) {
        std::string col = (i < app->signalLevel)
            ? (isViper ? "#00cfff" : "#4dff91") : "#2a4a2a";
        applyCSS(c, "box { background:" + col + "; }");
        c = gtk_widget_get_next_sibling(c);
        i++;
    }
}

static void updateActivityBars(bool isViper) {
    std::uniform_int_distribution<int> coin(0, 1);
    GtkWidget *c = gtk_widget_get_first_child(app->activityBox);
    while (c) {
        std::string col = coin(app->rng)
            ? (isViper ? "#00cfff" : "#4dff91") : "#2a4a2a";
        applyCSS(c, "box { background:" + col + "; }");
        c = gtk_widget_get_next_sibling(c);
    }
}

// ─────────────────────────────────────────────
//  Timer callbacks
// ─────────────────────────────────────────────

static gboolean onMessageTick(gpointer) {
    if (app->paused) return G_SOURCE_CONTINUE;

    std::uniform_real_distribution<double> d(0.0, 1.0);

    if (d(app->rng) < 0.10) {
        if (app->activeFilter == "all") {
            GtkWidget *noise = gtk_label_new(pick(NOISE, app->rng).c_str());
            gtk_label_set_xalign(GTK_LABEL(noise), 0.0f);
            applyCSS(noise,
                "label { color:#1a4a2a; font-size:11px;"
                " font-style:italic; padding:1px 8px; }");
            gtk_box_append(GTK_BOX(app->feed), noise);
            g_idle_add_once([](gpointer) { scrollFeedToBottom(); }, nullptr);
            g_timeout_add(1800, [](gpointer w) -> gboolean {
                GtkWidget *widget = GTK_WIDGET(w);
                GtkWidget *parent = gtk_widget_get_parent(widget);
                if (parent) gtk_box_remove(GTK_BOX(parent), widget);
                return G_SOURCE_REMOVE;
            }, noise);
        }
        return G_SOURCE_CONTINUE;
    }

    Message msg = generateMessage();
    app->messages.push_back(msg);
    if (app->messages.size() > 200)
        app->messages.erase(app->messages.begin());

    app->txCount++;
    appendMessageToFeed(msg);
    updateSignalBars(msg.isAir);
    updateActivityBars(msg.isAir);

    gtk_label_set_text(GTK_LABEL(app->msgCount),
        (std::to_string(app->txCount) + " transmissions").c_str());
    gtk_label_set_text(GTK_LABEL(app->txLabel),
        std::to_string(app->txCount).c_str());
    gtk_label_set_text(GTK_LABEL(app->anomalyLabel),
        std::to_string(app->anomalyCount).c_str());

    return G_SOURCE_CONTINUE;
}

static gboolean onClockTick(gpointer) {
    gtk_label_set_text(GTK_LABEL(app->clockLabel), currentTime().c_str());
    gtk_label_set_text(GTK_LABEL(app->uptimeLabel), uptimeStr().c_str());

    std::uniform_int_distribution<int> da(-15, 15), ds(-10, 10);
    for (auto& v : app->vipers) {
        if (v.viperStatus == "AIRBORNE") {
            v.alt = std::max(100, std::min(500, v.alt + da(app->rng)));
            v.spd = std::max(150, std::min(280, v.spd + ds(app->rng)));
        }
    }
    return G_SOURCE_CONTINUE;
}

static gboolean onActivityTick(gpointer) {
    updateActivityBars(false);
    return G_SOURCE_CONTINUE;
}

// ─────────────────────────────────────────────
//  UI callbacks
// ─────────────────────────────────────────────

static void onFilterClicked(GtkButton *btn, gpointer data) {
    app->activeFilter = (const char*)data;
    GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(btn));
    GtkWidget *child  = gtk_widget_get_first_child(parent);
    while (child) {
        gtk_widget_remove_css_class(child, "btn-active");
        child = gtk_widget_get_next_sibling(child);
    }
    gtk_widget_add_css_class(GTK_WIDGET(btn), "btn-active");
    rebuildFeed();
}

static void onPauseClicked(GtkButton *btn, gpointer) {
    app->paused = !app->paused;
    gtk_button_set_label(btn, app->paused ? "Resume" : "Pause");
    if (app->paused) gtk_widget_add_css_class(GTK_WIDGET(btn), "btn-active");
    else             gtk_widget_remove_css_class(GTK_WIDGET(btn), "btn-active");
}

static void onClearClicked(GtkButton*, gpointer) {
    app->messages.clear();
    app->txCount = 0;
    gtk_label_set_text(GTK_LABEL(app->msgCount), "0 transmissions");
    gtk_label_set_text(GTK_LABEL(app->txLabel),  "0");
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(app->feed)) != nullptr)
        gtk_box_remove(GTK_BOX(app->feed), child);
}

static void onSquadClicked(GtkButton*, gpointer data) {
    Squad *sq = static_cast<Squad*>(data);
    gtk_label_set_text(GTK_LABEL(app->freqLabel), (sq->freq + " MHz").c_str());
    gtk_widget_set_visible(app->altRow, false);
}

static void onViperClicked(GtkButton*, gpointer data) {
    Squad *v = static_cast<Squad*>(data);
    gtk_label_set_text(GTK_LABEL(app->freqLabel), (v->freq + " MHz").c_str());
    bool airborne = (v->viperStatus == "AIRBORNE");
    gtk_widget_set_visible(app->altRow, airborne);
    if (airborne) {
        gtk_label_set_text(GTK_LABEL(app->altLabel),
            (std::to_string(v->alt) + "m").c_str());
        gtk_label_set_text(GTK_LABEL(app->spdLabel),
            (std::to_string(v->spd) + "km/h").c_str());
    }
}

// ─────────────────────────────────────────────
//  UI builders
// ─────────────────────────────────────────────

static GtkWidget* makeLabel(const std::string& text, const std::string& cls = "") {
    GtkWidget *l = gtk_label_new(text.c_str());
    gtk_label_set_xalign(GTK_LABEL(l), 0.0f);
    if (!cls.empty()) gtk_widget_add_css_class(l, cls.c_str());
    return l;
}

static GtkWidget* makeFilterBtn(const std::string& label,
                                 const char* filter, bool active = false)
{
    GtkWidget *btn = gtk_button_new_with_label(label.c_str());
    gtk_widget_add_css_class(btn, "filter-btn");
    if (active) gtk_widget_add_css_class(btn, "btn-active");
    static std::vector<std::string> store;
    store.push_back(filter);
    g_signal_connect(btn, "clicked",
        G_CALLBACK(onFilterClicked), (gpointer)store.back().c_str());
    return btn;
}

static GtkWidget* buildSidebar() {
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_size_request(sidebar, 240, -1);

    // Ground squads
    {
        GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_add_css_class(panel, "panel");
        gtk_box_append(GTK_BOX(panel), makeLabel("Ground Squads", "panel-label"));

        for (auto& sq : app->squads) {
            GtkWidget *btn = gtk_button_new();
            gtk_widget_add_css_class(btn, "squad-btn");
            GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
            gtk_widget_set_halign(row, GTK_ALIGN_FILL);

            GtkWidget *dot = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_widget_set_size_request(dot, 8, 8);
            applyCSS(dot, "box { background:" + sq.color +
                "; border-radius:50%; min-width:8px; min-height:8px; }");
            gtk_box_append(GTK_BOX(row), dot);

            GtkWidget *name = gtk_label_new(sq.name.c_str());
            gtk_label_set_xalign(GTK_LABEL(name), 0.0f);
            gtk_widget_set_hexpand(name, TRUE);
            gtk_widget_add_css_class(name, "squad-name");
            gtk_box_append(GTK_BOX(row), name);

            GtkWidget *cnt = gtk_label_new(std::to_string(sq.members).c_str());
            gtk_widget_add_css_class(cnt, "squad-count");
            gtk_box_append(GTK_BOX(row), cnt);

            gtk_button_set_child(GTK_BUTTON(btn), row);
            g_signal_connect(btn, "clicked", G_CALLBACK(onSquadClicked), &sq);
            gtk_box_append(GTK_BOX(panel), btn);
        }
        gtk_box_append(GTK_BOX(sidebar), panel);
    }

    // VIPER air wing
    {
        GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_add_css_class(panel, "panel");
        gtk_box_append(GTK_BOX(panel), makeLabel("VIPER Air Wing", "panel-label-viper"));

        for (auto& v : app->vipers) {
            GtkWidget *btn = gtk_button_new();
            gtk_widget_add_css_class(btn, "squad-btn");
            GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

            bool airborne = (v.viperStatus == "AIRBORNE");
            std::string sc = airborne ? "#00cfff" : "#ffb347";

            GtkWidget *icon = gtk_label_new("[H]");
            applyCSS(icon, "label { color:" + sc + "; font-size:11px; }");
            gtk_box_append(GTK_BOX(row), icon);

            GtkWidget *name = gtk_label_new(v.name.c_str());
            gtk_label_set_xalign(GTK_LABEL(name), 0.0f);
            gtk_widget_set_hexpand(name, TRUE);
            gtk_widget_add_css_class(name, "viper-name");
            gtk_box_append(GTK_BOX(row), name);

            GtkWidget *status = gtk_label_new(v.viperStatus.c_str());
            applyCSS(status, "label { color:" + sc + "; font-size:9px; }");
            gtk_box_append(GTK_BOX(row), status);

            gtk_button_set_child(GTK_BUTTON(btn), row);
            g_signal_connect(btn, "clicked", G_CALLBACK(onViperClicked), &v);
            gtk_box_append(GTK_BOX(panel), btn);
        }
        gtk_box_append(GTK_BOX(sidebar), panel);
    }

    // Signal panel
    {
        GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
        gtk_widget_add_css_class(panel, "panel");
        gtk_box_append(GTK_BOX(panel), makeLabel("Signal", "panel-label"));

        app->freqLabel = gtk_label_new("100.9 MHz");
        gtk_widget_add_css_class(app->freqLabel, "freq-display");
        gtk_label_set_xalign(GTK_LABEL(app->freqLabel), 0.5f);
        gtk_box_append(GTK_BOX(panel), app->freqLabel);

        GtkWidget *sigRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_halign(sigRow, GTK_ALIGN_END);
        app->signalBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
        gtk_widget_set_valign(app->signalBox, GTK_ALIGN_END);
        int heights[] = {6, 9, 12, 15, 18};
        for (int i = 0; i < 5; i++) {
            GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            gtk_widget_set_size_request(bar, 5, heights[i]);
            std::string c = (i < app->signalLevel) ? "#4dff91" : "#2a4a2a";
            applyCSS(bar, "box { background:" + c + "; }");
            gtk_box_append(GTK_BOX(app->signalBox), bar);
        }
        gtk_box_append(GTK_BOX(sigRow), app->signalBox);
        gtk_box_append(GTK_BOX(panel), sigRow);

        app->altRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_set_visible(app->altRow, false);
        GtkWidget *altBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        gtk_box_append(GTK_BOX(altBox), makeLabel("ALT", "stat-key"));
        app->altLabel = makeLabel("---m", "viper-val");
        gtk_box_append(GTK_BOX(altBox), app->altLabel);
        GtkWidget *spdBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        gtk_box_append(GTK_BOX(spdBox), makeLabel("SPD", "stat-key"));
        app->spdLabel = makeLabel("---km/h", "viper-val");
        gtk_box_append(GTK_BOX(spdBox), app->spdLabel);
        gtk_box_append(GTK_BOX(app->altRow), altBox);
        gtk_box_append(GTK_BOX(app->altRow), spdBox);
        gtk_box_append(GTK_BOX(panel), app->altRow);

        app->activityBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
        gtk_widget_set_size_request(app->activityBox, -1, 3);
        for (int i = 0; i < 8; i++) {
            GtkWidget *seg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_widget_set_hexpand(seg, TRUE);
            applyCSS(seg, "box { background:#2a4a2a; }");
            gtk_box_append(GTK_BOX(app->activityBox), seg);
        }
        gtk_box_append(GTK_BOX(panel), app->activityBox);
        gtk_box_append(GTK_BOX(sidebar), panel);
    }

    // Filters
    {
        GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
        gtk_widget_add_css_class(panel, "panel");
        gtk_box_append(GTK_BOX(panel), makeLabel("Filters", "panel-label"));

        GtkWidget *g1 = gtk_grid_new();
        gtk_grid_set_column_spacing(GTK_GRID(g1), 4);
        gtk_grid_set_row_spacing(GTK_GRID(g1), 4);
        GtkWidget *bAll     = makeFilterBtn("All",     "all",     true);
        GtkWidget *bAir     = makeFilterBtn("Air",     "air");
        GtkWidget *bDanger  = makeFilterBtn("Danger",  "danger");
        GtkWidget *bChatter = makeFilterBtn("Chatter", "chatter");
        gtk_widget_set_hexpand(bAll,     TRUE); gtk_widget_set_hexpand(bAir,     TRUE);
        gtk_widget_set_hexpand(bDanger,  TRUE); gtk_widget_set_hexpand(bChatter, TRUE);
        gtk_grid_attach(GTK_GRID(g1), bAll,     0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(g1), bAir,     1, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(g1), bDanger,  0, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(g1), bChatter, 1, 1, 1, 1);
        gtk_box_append(GTK_BOX(panel), g1);

        GtkWidget *g2 = gtk_grid_new();
        gtk_grid_set_column_spacing(GTK_GRID(g2), 4);
        app->pauseBtn = gtk_button_new_with_label("Pause");
        gtk_widget_add_css_class(app->pauseBtn, "filter-btn");
        gtk_widget_set_hexpand(app->pauseBtn, TRUE);
        g_signal_connect(app->pauseBtn, "clicked", G_CALLBACK(onPauseClicked), nullptr);
        GtkWidget *clearBtn = gtk_button_new_with_label("Clear");
        gtk_widget_add_css_class(clearBtn, "filter-btn");
        gtk_widget_add_css_class(clearBtn, "btn-danger");
        gtk_widget_set_hexpand(clearBtn, TRUE);
        g_signal_connect(clearBtn, "clicked", G_CALLBACK(onClearClicked), nullptr);
        gtk_grid_attach(GTK_GRID(g2), app->pauseBtn, 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(g2), clearBtn,      1, 0, 1, 1);
        gtk_box_append(GTK_BOX(panel), g2);
        gtk_box_append(GTK_BOX(sidebar), panel);
    }

    return sidebar;
}

static GtkWidget* buildMainFeed() {
    GtkWidget *col = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_hexpand(col, TRUE);

    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(header, "feed-header");
    GtkWidget *title = makeLabel("# Radio Log - Ground + Air Transmissions", "feed-title");
    gtk_widget_set_hexpand(title, TRUE);
    gtk_box_append(GTK_BOX(header), title);
    app->msgCount = makeLabel("0 transmissions", "feed-count");
    gtk_box_append(GTK_BOX(header), app->msgCount);
    gtk_box_append(GTK_BOX(col), header);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_add_css_class(scroll, "feed-scroll");

    app->feed = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(app->feed, "feed-box");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), app->feed);
    gtk_box_append(GTK_BOX(col), scroll);

    return col;
}

static GtkWidget* buildFooter() {
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(bar, "footer");

    auto addStat = [&](const std::string& key, GtkWidget** valOut,
                       const std::string& init = "0")
    {
        GtkWidget *cell = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_box_append(GTK_BOX(cell), makeLabel(key, "stat-key"));
        GtkWidget *v = makeLabel(init, "stat-val");
        gtk_box_append(GTK_BOX(cell), v);
        if (valOut) *valOut = v;
        gtk_box_append(GTK_BOX(bar), cell);
        GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        gtk_widget_add_css_class(sep, "footer-sep");
        gtk_box_append(GTK_BOX(bar), sep);
    };

    addStat("GROUND SQUADS",  nullptr, std::to_string(app->squads.size()));
    int ab = 0;
    for (auto& v : app->vipers) if (v.viperStatus == "AIRBORNE") ab++;
    addStat("VIPERS AIRBORNE", nullptr, std::to_string(ab));
    addStat("ANOMALIES",  &app->anomalyLabel);
    addStat("TX COUNT",   &app->txLabel);
    addStat("UPTIME",     &app->uptimeLabel);

    return bar;
}

// ─────────────────────────────────────────────
//  Global CSS
// ─────────────────────────────────────────────

static const char APP_CSS[] =
"window { background-color:#0a120a; color:#3aff7a; }\n"
"* { font-family:'Courier New',Courier,monospace; font-size:13px; color:#3aff7a; }\n"
".header-box { background:#0d1a0d; border-bottom:1px solid #2a4a2a; padding:10px 16px; }\n"
".header-title { font-size:14px; font-weight:bold; color:#ffb347; letter-spacing:2px; }\n"
".header-status { font-size:11px; color:#2a7a4a; }\n"
".clock-label { font-size:11px; color:#ffb347; }\n"
".clock-sub { font-size:10px; color:#2a7a4a; }\n"
".panel { background:#0d1a0d; border:1px solid #2a4a2a; padding:10px; }\n"
".panel-label { font-size:10px; letter-spacing:1.5px; color:#2a7a4a;"
" border-bottom:1px solid #2a4a2a; padding-bottom:6px; margin-bottom:8px; }\n"
".panel-label-viper { font-size:10px; letter-spacing:1.5px; color:#00cfff;"
" border-bottom:1px solid #003a50; padding-bottom:6px; margin-bottom:8px; }\n"
".squad-btn { background:transparent; border:1px solid transparent;"
" border-radius:0; padding:5px 6px; margin-bottom:2px; }\n"
".squad-btn:hover { border-color:#2a4a2a; }\n"
".squad-name { font-size:12px; }\n"
".squad-count { font-size:10px; color:#2a7a4a; }\n"
".viper-name { font-size:12px; color:#aaeeff; }\n"
".freq-display { font-size:20px; color:#ffb347; letter-spacing:2px;"
" background:#0c1a0c; border:1px solid #2a4a2a; padding:8px; font-weight:bold; }\n"
".filter-btn { background:transparent; border:1px solid #2a4a2a; border-radius:0;"
" color:#2a7a4a; font-family:'Courier New',monospace; font-size:10px;"
" letter-spacing:1px; padding:5px 4px; }\n"
".filter-btn:hover { border-color:#4dff91; color:#3aff7a; }\n"
".filter-btn.btn-active { border-color:#ffb347; color:#ffb347; background:#1a1200; }\n"
".filter-btn.btn-danger { color:#ff5555; }\n"
".filter-btn.btn-danger:hover { border-color:#ff5555; }\n"
".feed-header { background:#0d1a0d; border:1px solid #2a4a2a; padding:8px 12px; }\n"
".feed-title { font-size:11px; letter-spacing:1px; color:#2a7a4a; }\n"
".feed-count { font-size:11px; color:#ffb347; }\n"
".feed-scroll { background:#0d1a0d; border:1px solid #2a4a2a; }\n"
".feed-box { background:#0d1a0d; padding:8px; }\n"
".msg-time { font-size:10px; color:#2a7a4a; }\n"
".msg-freq { font-size:10px; color:#2a7a4a; }\n"
".footer { background:#0d1a0d; border-top:1px solid #2a4a2a; padding:8px 12px; }\n"
".stat-key { font-size:10px; color:#2a7a4a; }\n"
".stat-val { font-size:10px; color:#3aff7a; }\n"
".viper-val { font-size:10px; color:#00cfff; }\n"
".footer-sep { background:#2a4a2a; min-width:1px; }\n"
"scrollbar { background:#0a120a; }\n"
"scrollbar slider { background:#2a4a2a; border-radius:0; min-width:4px; min-height:4px; }\n";

// ─────────────────────────────────────────────
//  Data init
// ─────────────────────────────────────────────

static void initData() {
    app->squads = {
        {"loner1",   "Wolf Pack",    "loner",     "100.9", "#4dff91", 4, false, "", "", 0, 0},
        {"duty1",    "Iron Fist",    "duty",      "103.4", "#ff5555", 5, false, "", "", 0, 0},
        {"freedom1", "Free Wind",    "freedom",   "98.7",  "#ffb347", 3, false, "", "", 0, 0},
        {"merc1",    "Cold Steel",   "mercs",     "107.1", "#88aaff", 6, false, "", "", 0, 0},
        {"bandit1",  "The Jackals",  "bandit",    "94.2",  "#cc66aa", 4, false, "", "", 0, 0},
        {"eco1",     "Green Shield", "ecologist", "111.5", "#aaddaa", 2, false, "", "", 0, 0},
    };
    app->vipers = {
        {"v1","VIPER-1","viper","121.5","#00cfff",0,true,"Attack", "AIRBORNE",320,240},
        {"v2","VIPER-2","viper","121.5","#00cfff",0,true,"Scout",  "AIRBORNE",180,190},
        {"v3","VIPER-3","viper","121.5","#00cfff",0,true,"Gunship","AIRBORNE",410,210},
        {"v4","VIPER-4","viper","123.1","#ffb347",0,true,"Medevac","RTB",       0,  0},
        {"v5","VIPER-5","viper","121.5","#00cfff",0,true,"Attack", "AIRBORNE",290,225},
    };
}

// ─────────────────────────────────────────────
//  Activate
// ─────────────────────────────────────────────

static void activate(GtkApplication *gtkApp, gpointer) {
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css, APP_CSS);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css);

    GtkWidget *window = gtk_application_window_new(gtkApp);
    gtk_window_set_title(GTK_WINDOW(window), "STALKER: Anomaly - PDA Radio Scanner");
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 700);

    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), root);

    // Header
    GtkWidget *headerBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(headerBox, "header-box");
    GtkWidget *leftHead = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_hexpand(leftHead, TRUE);
    GtkWidget *titleLbl = gtk_label_new("# ANOMALY - PDA RADIO SCANNER v2.5.0");
    gtk_label_set_xalign(GTK_LABEL(titleLbl), 0.0f);
    gtk_widget_add_css_class(titleLbl, "header-title");
    GtkWidget *statusLbl = gtk_label_new("ZONE COMMS + AIR TRAFFIC INTERCEPTOR  |  ACTIVE");
    gtk_label_set_xalign(GTK_LABEL(statusLbl), 0.0f);
    gtk_widget_add_css_class(statusLbl, "header-status");
    gtk_box_append(GTK_BOX(leftHead), titleLbl);
    gtk_box_append(GTK_BOX(leftHead), statusLbl);
    gtk_box_append(GTK_BOX(headerBox), leftHead);

    GtkWidget *rightHead = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_halign(rightHead, GTK_ALIGN_END);
    gtk_widget_set_valign(rightHead, GTK_ALIGN_CENTER);
    app->clockLabel = gtk_label_new("00:00:00");
    gtk_widget_add_css_class(app->clockLabel, "clock-label");
    gtk_label_set_xalign(GTK_LABEL(app->clockLabel), 1.0f);
    GtkWidget *clockSub = gtk_label_new("ZONE LOCAL TIME");
    gtk_widget_add_css_class(clockSub, "clock-sub");
    gtk_label_set_xalign(GTK_LABEL(clockSub), 1.0f);
    gtk_box_append(GTK_BOX(rightHead), app->clockLabel);
    gtk_box_append(GTK_BOX(rightHead), clockSub);
    gtk_box_append(GTK_BOX(headerBox), rightHead);
    gtk_box_append(GTK_BOX(root), headerBox);

    // Content
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_vexpand(content, TRUE);
    gtk_widget_set_margin_start(content, 16);
    gtk_widget_set_margin_end(content, 16);
    gtk_widget_set_margin_top(content, 10);
    gtk_widget_set_margin_bottom(content, 10);
    gtk_box_append(GTK_BOX(content), buildSidebar());
    gtk_box_append(GTK_BOX(content), buildMainFeed());
    gtk_box_append(GTK_BOX(root), content);
    gtk_box_append(GTK_BOX(root), buildFooter());

    gtk_window_present(GTK_WINDOW(window));

    g_timeout_add(900,  onMessageTick,  nullptr);
    g_timeout_add(1000, onClockTick,    nullptr);
    g_timeout_add(500,  onActivityTick, nullptr);

    for (int i = 0; i < 10; i++) onMessageTick(nullptr);
}

// ─────────────────────────────────────────────
//  Entry point
// ─────────────────────────────────────────────

int main(int argc, char *argv[]) {
    app = new AppState();
    initData();

    GtkApplication *gtkApp = gtk_application_new(
        "io.github.stalker.radio", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(gtkApp, "activate", G_CALLBACK(activate), nullptr);

    int status = g_application_run(G_APPLICATION(gtkApp), argc, argv);
    g_object_unref(gtkApp);
    delete app;
    return status;
}

// Feedback page — a port of crates/gallery/src/sections/feedback.rs.
#include <algorithm>
#include <cstdio>
#include <tuple>

#include "../gallery.hpp"

namespace gallery {

namespace {

/// Small local layout helpers for this page.
const char* const kFeedbackCss = R"(
.fb-stack { display: grid; gap: 10px; }
.fb-alerts { display: grid; grid-template-columns: repeat(auto-fill, minmax(380px, 1fr)); gap: 10px; margin-bottom: 18px; }
.fb-alerts:last-child { margin-bottom: 0; }
.fb-variant { margin: 0 0 8px; }
.fb-linear { display: grid; grid-template-columns: 92px 1fr 44px; align-items: center; gap: 14px; margin-bottom: 14px; }
.fb-linear:last-child { margin-bottom: 0; }
.fb-value { position: relative; display: inline-flex; align-items: center; justify-content: center; }
.fb-value > span.skeuo-caption { position: absolute; font-size: 11px; font-weight: 600; color: var(--sk-ink-2); }
.fb-card { padding: 16px; display: grid; gap: 12px; }
.fb-card__head { display: flex; align-items: center; gap: 12px; }
.fb-card__head > div { flex: 1; }
.fb-stage { position: relative; display: flex; align-items: center; justify-content: center; height: 170px; padding: 16px; overflow: hidden; }
.fb-dialogs { display: grid; grid-template-columns: repeat(auto-fill, minmax(380px, 1fr)); gap: 20px; margin-bottom: 18px; }
.fb-snacks { display: flex; flex-wrap: wrap; gap: 16px; margin-bottom: 16px; }
.fb-modal-panel { padding: 22px 24px; width: 360px; max-width: 100%; }
.fb-modal-panel h3 { margin: 0 0 6px; font-size: 17px; letter-spacing: -.015em; }
.fb-modal-panel p { margin: 0 0 16px; }
.fb-icon-btn { min-width: 0 !important; padding: 0 7px !important; }
)";

/// A number as Rust's `{}` prints an f64 ("40", "12.5").
std::string fmt(double v) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%g", v);
    return buf;
}

Node AlertDemo() {
    bool& dismissed = use_state<bool>("fb.dismissed", [] { return false; });
    Node variants;
    for (AlertVariant variant : kAllAlertVariants) {
        variants.add_child(el("p").cls("skeuo-label fb-variant").child(std::string(as_str(variant))));
        Node alerts = el("div").cls("fb-alerts");
        for (AlertSeverity severity : kAllAlertSeverities)
            alerts.child(Alert({.severity = severity, .variant = variant},
                               "This is a " + std::string(as_str(severity)) + " alert — check it out."));
        variants.add_child(std::move(alerts));
    }
    Node dismissible =
        dismissed ? Button({.variant = ButtonVariant::Outlined,
                            .color = Color::Neutral,
                            .onclick = [&dismissed] { dismissed = false; }},
                           "Bring the alert back")
                  : Alert({.severity = AlertSeverity::Warning,
                           .variant = AlertVariant::Outlined,
                           .onclose = [&dismissed] { dismissed = true; }},
                          "Dismiss me with the close button.");
    return Demo(
        "Alert",
        "standard = aluminium washed with tint · filled = flat colored plate · outlined = colored hairline edge",
        {
            std::move(variants),
            el("p").cls("skeuo-label fb-variant").child("title, action, close, custom icon"),
            el("div").cls("fb-alerts").children({
                Alert({.severity = AlertSeverity::Error},
                      {AlertTitle({}, "Stream offline"), "The encoder lost its connection at 14:02."}),
                Alert({.severity = AlertSeverity::Info,
                       .action = Button({.variant = ButtonVariant::Text, .color = Color::Info, .size = Size::Small},
                                        "Retry")},
                      "Reconnecting to the studio…"),
                std::move(dismissible),
                Alert({.severity = AlertSeverity::Success, .variant = AlertVariant::Filled,
                       .icon = Icon(IconName::Mic)},
                      "On air: custom icon."),
                Alert({.severity = AlertSeverity::Info, .hide_icon = true}, "No icon, just the message."),
            }),
        });
}

Node ProgressDemo() {
    double& progress = use_state<double>("fb.progress", [] { return 40.0; });

    Node indeterminate;
    for (Color c : {Color::Primary, Color::Secondary, Color::Success, Color::Error, Color::Warning})
        indeterminate.add_child(CircularProgress({.color = c, .aria_label = "Loading"}));
    indeterminate.add_child(CircularProgress({.disable_shrink = true, .aria_label = "Loading"}));
    indeterminate.add_child(
        CircularProgress({.size = 24, .thickness = 4.5, .color = Color::Info, .aria_label = "Loading"}));

    Node determinate;
    for (auto [value, color] : {std::pair{0.0, Color::Primary}, std::pair{25.0, Color::Primary},
                                std::pair{50.0, Color::Success}, std::pair{75.0, Color::Warning},
                                std::pair{100.0, Color::Error}}) {
        determinate.add_child(el("span").cls("fb-value").children({
            CircularProgress({.variant = CircularProgressVariant::Determinate, .value = value, .size = 52,
                              .thickness = 4.0, .color = color}),
            el("span").cls("skeuo-caption").child(fmt(value) + "%"),
        }));
    }
    determinate.add_child(CircularProgress({.variant = CircularProgressVariant::Determinate, .value = 66.0,
                                            .size = 28, .thickness = 5.0, .color = Color::Info}));
    determinate.add_child(CircularProgress({.variant = CircularProgressVariant::Determinate, .value = 85.0,
                                            .size = 72, .thickness = 3.0, .color = Color::Secondary}));

    auto linear_row = [](std::string label, Node bar, Node tail) {
        return el("div").cls("fb-linear").children({
            el("span").cls("demo__row-label").child(std::move(label)),
            std::move(bar),
            std::move(tail),
        });
    };
    Node linears;
    for (auto [label, value, color] :
         {std::tuple{"determinate", 30.0, Color::Primary}, std::tuple{"success", 62.0, Color::Success},
          std::tuple{"warning", 88.0, Color::Warning}, std::tuple{"error", 12.0, Color::Error}}) {
        linears.add_child(linear_row(
            label, LinearProgress({.variant = LinearProgressVariant::Determinate, .value = value, .color = color}),
            el("span").cls("skeuo-caption").child(fmt(value) + "%")));
    }

    return Demo(
        "Progress",
        "CircularProgress = ring groove with a colored arc · LinearProgress = groove with a colored fill",
        {
            DemoRow("indeterminate", std::move(indeterminate)),
            DemoRow("determinate", std::move(determinate)),
            el("div").style("height: 8px"),
            std::move(linears),
            linear_row("buffer",
                       LinearProgress({.variant = LinearProgressVariant::Buffer, .value = 45.0,
                                       .value_buffer = 70.0, .color = Color::Info}),
                       el("span").cls("skeuo-caption").child("45%")),
            linear_row("indeterminate", LinearProgress({.aria_label = "Loading"}), el("span")),
            linear_row("query",
                       LinearProgress({.variant = LinearProgressVariant::Query, .color = Color::Secondary,
                                       .aria_label = "Preparing"}),
                       el("span")),
            linear_row("interactive",
                       LinearProgress({.variant = LinearProgressVariant::Determinate, .value = progress,
                                       .color = Color::Success}),
                       el("span").cls("skeuo-caption").child(fmt(progress) + "%")),
            // Rust's DemoRow { label: "" } still renders an (empty) label.
            el("div").cls("demo__row").children({
                el("span").cls("demo__row-label"),
                Button({.color = Color::Success,
                        .size = Size::Small,
                        .onclick = [&progress] { progress = progress >= 100.0 ? 0.0 : std::min(progress + 10.0, 100.0); }},
                       "Advance 10%"),
                CircularProgress({.variant = CircularProgressVariant::Determinate, .value = progress, .size = 28,
                                  .color = Color::Success}),
            }),
        });
}

Node SkeletonDemo() {
    using V = SkeletonVariant;
    using A = SkeletonAnimation;
    return Demo(
        "Skeleton",
        "shallow wells pressed where content will land · text / rectangular / rounded / circular · pulse / wave",
        DemoGrid({
            el("div").cls("skeuo-chassis skeuo-elev-1 fb-card").children({
                el("div").cls("fb-card__head").children({
                    Skeleton({.variant = V::Circular, .width = "40px", .height = "40px"}),
                    el("div").children({Skeleton({.width = "70%"}), Skeleton({.width = "40%"})}),
                }),
                Skeleton({.variant = V::Rounded, .height = "110px"}),
                el("div").children({Skeleton({}), Skeleton({}), Skeleton({.width = "60%"})}),
            }),
            el("div").cls("skeuo-chassis skeuo-elev-1 fb-card").children({
                el("div").cls("fb-card__head").children({
                    Skeleton({.variant = V::Circular, .animation = A::Wave, .width = "40px", .height = "40px"}),
                    el("div").children({Skeleton({.animation = A::Wave, .width = "80%"}),
                                        Skeleton({.animation = A::Wave, .width = "50%"})}),
                }),
                Skeleton({.variant = V::Rectangular, .animation = A::Wave, .height = "110px"}),
                Skeleton({.animation = A::Wave}),
                Skeleton({.animation = A::Wave, .width = "75%"}),
            }),
            el("div").cls("fb-stack").children({
                el("span").cls("skeuo-label").child("variants (animation = false)"),
                Skeleton({.animation = A::False, .width = "100%"}),
                Skeleton({.variant = V::Rectangular, .animation = A::False, .width = "100%", .height = "40px"}),
                Skeleton({.variant = V::Rounded, .animation = A::False, .width = "100%", .height = "40px"}),
                Skeleton({.variant = V::Circular, .animation = A::False, .width = "40px", .height = "40px"}),
                el("span").cls("skeuo-label").child("sized by children"),
                Skeleton({.variant = V::Rounded}, Button({}, "Hidden button")),
            }),
        }));
}

Node DialogDemo() {
    bool& open = use_state<bool>("fb.dialog.open", [] { return false; });
    std::string& answer = use_state<std::string>("fb.dialog.answer", [] { return std::string("none yet"); });
    std::string& title = use_state<std::string>("fb.dialog.title", [] { return std::string("Evening news"); });
    return Demo(
        "Dialog",
        "a level-4 floating chassis over the scrim · actions are right-aligned keycaps · previews shown in flow",
        {
            el("div").cls("fb-dialogs").children({
                Dialog({.open = true, .max_width = DialogMaxWidth::Xs, .inline_ = true},
                       {
                           DialogTitle({}, "Use location service?"),
                           DialogContent({}, DialogContentText({}, "Let the app determine your location. Anonymous "
                                                                   "data is sent to the station, even when no apps "
                                                                   "are running.")),
                           DialogActions({}, {
                               Button({.variant = ButtonVariant::Outlined, .color = Color::Neutral}, "Disagree"),
                               Button({}, "Agree"),
                           }),
                       }),
                Dialog({.open = true, .full_width = true, .inline_ = true},
                       {
                           DialogTitle({}, "Schedule announcement"),
                           DialogContent({.dividers = true},
                                         {
                                             DialogContentText({}, "Pick a slot. The clip plays at the top of the hour."),
                                             // Controlled rather than `default_value` (as in Rust):
                                             // positional hook slots would hand this field the
                                             // default of the Inputs page's first field after
                                             // navigating here.
                                             TextField({.label = "Title",
                                                        .value = title,
                                                        .full_width = true,
                                                        .oninput = [&title](const std::string& v) { title = v; }}),
                                             el("div").style("height: 12px"),
                                             LinearProgress({.variant = LinearProgressVariant::Determinate,
                                                             .value = 72.0}),
                                         }),
                           DialogActions({}, {
                               Button({.variant = ButtonVariant::Text}, "Cancel"),
                               Button({.color = Color::Success}, "Schedule"),
                           }),
                       }),
            }),
            DemoRow("interactive", {
                Button({.onclick = [&open] { open = true; }}, "Open dialog"),
                el("span").cls("skeuo-caption").child("Last answer: " + answer),
            }),
            Dialog({.open = open,
                    .onclose =
                        [&open, &answer](ModalCloseReason reason) {
                            answer = "dismissed (" + std::string(as_str(reason)) + ")";
                            open = false;
                        },
                    .aria_describedby = "fb-dialog-desc"},
                   {
                       DialogTitle({}, "Discard draft?"),
                       DialogContent({}, DialogContentText({.id = "fb-dialog-desc"},
                                                           "Your unsaved script will be lost. Press Escape or click "
                                                           "the scrim to keep editing.")),
                       DialogActions({}, {
                           Button({.variant = ButtonVariant::Outlined,
                                   .color = Color::Neutral,
                                   .onclick = [&open, &answer] { answer = "kept"; open = false; }},
                                  "Keep editing"),
                           Button({.color = Color::Error,
                                   .onclick = [&open, &answer] { answer = "discarded"; open = false; }},
                                  "Discard"),
                       }),
                   }),
        });
}

Node SnackbarDemo() {
    bool& open = use_state<bool>("fb.snack.open", [] { return false; });
    SnackbarOrigin& origin = use_state<SnackbarOrigin>("fb.snack.origin", [] { return SnackbarOrigin{}; });
    std::string& closed_by = use_state<std::string>("fb.snack.closed", [] { return std::string("—"); });
    unsigned& shown = use_state<unsigned>("fb.snack.shown", [] { return 0u; });
    auto show = [&open, &origin, &shown](SnackbarOrigin o) {
        return [&open, &origin, &shown, o] {
            origin = o;
            ++shown;
            open = true;
        };
    };

    Node snackbar = Snackbar(
        {.open = open,
         .message = "Snackbar from the gallery — hides in 4 s.",
         .action = Button({.variant = ButtonVariant::Text,
                           .size = Size::Small,
                           .onclick = [&open, &closed_by] { closed_by = "action"; open = false; }},
                          "Dismiss"),
         .onclose =
             [&open, &closed_by](SnackbarCloseReason reason) {
                 closed_by = std::string(as_str(reason));
                 open = false;
             },
         .auto_hide_duration = 4000,
         .anchor_origin = origin});
    // Rust bumps the snackbar's key on every show so it remounts and its
    // countdown restarts. The page patches by position, so a comment
    // placeholder alternating in front of it makes the patcher replace the
    // element instead (the comment renders nothing).
    Node keyed = open && shown % 2 == 1 ? Node{raw_html("<!--fb-snack-->"), std::move(snackbar)} : std::move(snackbar);

    return Demo(
        "Snackbar",
        "a light level-3 floating chassis that rises in; auto-hide is a CSS countdown (paused on hover)",
        {
            el("div").cls("fb-snacks").children({
                Snackbar({.open = true, .message = "Clip saved to the library.", .inline_ = true}),
                Snackbar({.open = true,
                          .message = "Announcement deleted.",
                          .action = Node{
                              Button({.variant = ButtonVariant::Text, .size = Size::Small}, "Undo"),
                              Button({.variant = ButtonVariant::Text,
                                      .color = Color::Neutral,
                                      .size = Size::Small,
                                      .cls = "fb-icon-btn",
                                      .attrs = {{"aria-label", "Close"}}},
                                     Icon(IconName::Close)),
                          },
                          .inline_ = true}),
                Snackbar({.open = true, .inline_ = true},
                         Alert({.severity = AlertSeverity::Success, .variant = AlertVariant::Filled},
                               "Broadcast scheduled.")),
            }),
            DemoRow("interactive", {
                Button({.onclick = show(SnackbarOrigin{})}, "Bottom left"),
                Button({.variant = ButtonVariant::Outlined,
                        .onclick = show({SnackbarVertical::Top, SnackbarHorizontal::Center})},
                       "Top center"),
                Button({.variant = ButtonVariant::Outlined,
                        .onclick = show({SnackbarVertical::Bottom, SnackbarHorizontal::Right})},
                       "Bottom right"),
                el("span").cls("skeuo-caption").child("Last closed by: " + closed_by),
            }),
            std::move(keyed),
        });
}

Node BackdropModalDemo() {
    bool& backdrop = use_state<bool>("fb.backdrop", [] { return false; });
    bool& modal = use_state<bool>("fb.modal", [] { return false; });
    return Demo(
        "Backdrop & Modal",
        "the scrim dims the desk; a modal centers focusable content on it (Escape or a scrim click closes)",
        {
            DemoGrid({
                el("div").cls("skeuo-well fb-stage").children({
                    el("span").cls("skeuo-caption").child("Content under the backdrop"),
                    Backdrop({.open = true, .inline_ = true},
                             CircularProgress({.color = Color::Neutral, .aria_label = "Loading"})),
                }),
                el("div").cls("fb-stack").children({
                    el("span").cls("skeuo-label").child("interactive"),
                    DemoRow("", {
                        Button({.onclick = [&backdrop] { backdrop = true; }}, "Show backdrop"),
                        Button({.variant = ButtonVariant::Outlined, .onclick = [&modal] { modal = true; }},
                               "Open modal"),
                    }),
                    el("span").cls("skeuo-caption").child("Click the backdrop to dismiss it."),
                }),
            }),
            Backdrop({.open = backdrop, .onclick = [&backdrop] { backdrop = false; }},
                     CircularProgress({.size = 48, .color = Color::Neutral, .aria_label = "Loading"})),
            Modal({.open = modal, .onclose = [&modal](ModalCloseReason) { modal = false; },
                   .aria_labelledby = "fb-modal-title"},
                  el("div").cls("skeuo-chassis skeuo-chassis--float skeuo-elev-4 fb-modal-panel").children({
                      el("h3").attr("id", "fb-modal-title").child("A bare modal"),
                      el("p").cls("skeuo-caption").child("Modal is the primitive under Dialog: scrim, focus and "
                                                         "Escape handling, and nothing else."),
                      Button({.onclick = [&modal] { modal = false; }}, "Close"),
                  })),
        });
}

}  // namespace

Node FeedbackPage() {
    return {
        el("style").child(raw_html(kFeedbackCss)),
        AlertDemo(),
        ProgressDemo(),
        SkeletonDemo(),
        DialogDemo(),
        SnackbarDemo(),
        BackdropModalDemo(),
    };
}

}  // namespace gallery

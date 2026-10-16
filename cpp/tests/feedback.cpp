// Port of crates/skeuo-ui/tests/feedback.rs, plus handler wiring through Runtime.
#include <optional>

#include "skeuo/skeuo.hpp"
#include "test.hpp"

using namespace skeuo;

namespace {

/// The handler id of `type` on the tag containing `marker` (e.g. a class
/// attribute), from `data-sk-on="click:3,keydown:4!"`. -1 when missing.
int feedback_handler(const std::string& html, std::string_view marker, std::string_view type) {
    size_t at = html.find(marker);
    if (at == std::string::npos) return -1;
    size_t end = html.find('>', at);
    size_t on = html.find("data-sk-on=\"", at);
    if (on == std::string::npos || on > end) return -1;
    size_t start = on + 12;
    std::string spec = html.substr(start, html.find('"', start) - start);
    std::string want = std::string(type) + ":";
    size_t pos = spec.find(want);
    if (pos == std::string::npos) return -1;
    return std::stoi(spec.substr(pos + want.size()));
}

std::string feedback_event(int id, std::string_view type, std::string_view key = "") {
    return R"({"t":"event","h":[)" + std::to_string(id) + R"(],"e":{"type":")" + std::string(type) +
           R"(","key":")" + std::string(key) + R"("}})";
}

}  // namespace

// ---------------------------------------------------------------- Alert

TEST(feedback_alert_defaults) {
    std::string html = render_static([] { return Alert({}, "Saved"); });
    CONTAINS(html, R"(class="skeuo-chassis skeuo-elev-0 skeuo-alert skeuo-alert--standard skeuo-color-success")");
    CONTAINS(html, R"(role="alert")");
    CONTAINS(html, "skeuo-alert__icon");
    CONTAINS(html, "Saved");
    LACKS(html, "skeuo-alert__close");
}

TEST(feedback_alert_variants_and_severity_colors) {
    std::string html = render_static([] {
        return Node{
            Alert({.severity = AlertSeverity::Error, .variant = AlertVariant::Filled}, "a"),
            Alert({.severity = AlertSeverity::Warning, .variant = AlertVariant::Outlined}, "b"),
            Alert({.severity = AlertSeverity::Info, .color = Color::Secondary}, "c"),
        };
    });
    CONTAINS(html, "skeuo-key skeuo-key--filled skeuo-alert skeuo-alert--filled skeuo-color-error");
    CONTAINS(html, "skeuo-alert--outlined skeuo-color-warning");
    CONTAINS(html, "skeuo-alert--standard skeuo-color-secondary");
}

TEST(feedback_alert_close_button_title_action_and_icon) {
    std::string html = render_static([] {
        return Alert({.onclose = [] {}, .cls = "mine"}, {AlertTitle({}, "Heads up"), "Body"});
    });
    for (const char* needle : {"skeuo-alert__close", R"(aria-label="Close")", R"(type="button")",
                               R"(class="skeuo-alert__title")", "Heads up", "mine"})
        CONTAINS(html, needle);

    html = render_static([] {
        return Alert({.hide_icon = true, .action = el("button").child("Undo"), .onclose = [] {}}, "x");
    });
    CONTAINS(html, "Undo");
    LACKS(html, "skeuo-alert__close");
    LACKS(html, "skeuo-alert__icon");
}

TEST(feedback_alert_custom_icon_and_close_text) {
    std::string html = render_static([] {
        return Alert({.icon = el("i").cls("my-icon"), .onclose = [] {}, .close_text = "Dismiss", .role = "status"},
                     "x");
    });
    CONTAINS(html, R"(<div class="skeuo-alert__icon" aria-hidden="true"><i class="my-icon"></i></div>)");
    CONTAINS(html, R"(aria-label="Dismiss" title="Dismiss")");
    CONTAINS(html, R"(role="status")");
}

TEST(feedback_alert_close_reaches_handler) {
    int closed = 0;
    Runtime rt([&] { return Alert({.onclose = [&] { ++closed; }}, "x"); });
    std::string html = rt.render();
    int id = feedback_handler(html, "skeuo-alert__close", "click");
    CHECK(id >= 0) << html;
    rt.handle(feedback_event(id, "click"));
    CHECK(closed == 1);
}

// ---------------------------------------------------------------- Backdrop / Modal

TEST(feedback_backdrop_open_closed_invisible_inline) {
    LACKS(render_static([] { return Backdrop({}, "x"); }), "skeuo-backdrop");
    std::string html =
        render_static([] { return Backdrop({.open = true, .invisible = true, .inline_ = true}, "spinner"); });
    CONTAINS(html, "skeuo-scrim skeuo-backdrop skeuo-backdrop--invisible skeuo-backdrop--inline");
    CONTAINS(html, "spinner");
}

TEST(feedback_backdrop_click_reaches_handler) {
    bool open = true;
    Runtime rt([&] { return Backdrop({.open = open, .onclick = [&] { open = false; }}, "x"); });
    std::string html = rt.render();
    rt.handle(feedback_event(feedback_handler(html, "skeuo-backdrop", "click"), "click"));
    CHECK(!open);
    LACKS(rt.render(), "skeuo-backdrop");
}

TEST(feedback_modal_renders_scrim_and_aria_modal_content) {
    std::string html = render_static([] {
        return Modal({.open = true, .aria_labelledby = "t"}, el("p").attr("id", "t").child("Hello"));
    });
    for (const char* needle : {R"(class="skeuo-modal")", R"(role="presentation")", "skeuo-scrim skeuo-modal__scrim",
                               R"(role="dialog")", R"(aria-modal="true")", R"(aria-labelledby="t")",
                               R"(tabindex="-1")", "data-sk-autofocus", "Hello"})
        CONTAINS(html, needle);
}

TEST(feedback_modal_closed_hide_backdrop_keep_mounted) {
    LACKS(render_static([] { return Modal({.open = false}, "x"); }), "skeuo-modal");
    std::string html = render_static([] { return Modal({.open = false, .keep_mounted = true}, "kept"); });
    CONTAINS(html, "skeuo-modal--hidden");
    CONTAINS(html, " hidden");
    CONTAINS(html, "kept");
    LACKS(html, "data-sk-autofocus");
    html = render_static([] { return Modal({.open = true, .hide_backdrop = true}, "x"); });
    LACKS(html, "skeuo-scrim");
}

TEST(feedback_modal_escape_and_scrim_report_reasons) {
    std::vector<ModalCloseReason> reasons;
    Runtime rt([&] {
        return Modal({.open = true, .onclose = [&](ModalCloseReason r) { reasons.push_back(r); }}, "x");
    });
    std::string html = rt.render();
    CONTAINS(html, R"(data-sk-keys="Escape")");
    int key = feedback_handler(html, "class=\"skeuo-modal\"", "keydown");
    CHECK(html.find("keydown:" + std::to_string(key) + "!") != std::string::npos) << html;
    rt.handle(feedback_event(key, "keydown", "Escape"));
    rt.handle(feedback_event(feedback_handler(html, "skeuo-modal__scrim", "click"), "click"));
    CHECK(reasons.size() == 2);
    CHECK(reasons.size() == 2 && reasons[0] == ModalCloseReason::EscapeKeyDown &&
          reasons[1] == ModalCloseReason::BackdropClick);
    CHECK(as_str(ModalCloseReason::BackdropClick) == "backdropClick");
    CHECK(as_str(ModalCloseReason::EscapeKeyDown) == "escapeKeyDown");
}

TEST(feedback_modal_disable_escape) {
    std::string html =
        render_static([] { return Modal({.open = true, .onclose = [](ModalCloseReason) {}, .disable_escape_key_down = true}, "x"); });
    LACKS(html, "data-sk-keys");
}

// ---------------------------------------------------------------- Dialog

TEST(feedback_dialog_closed_renders_nothing) {
    std::string html = render_static([] { return Dialog({.open = false}, DialogTitle({}, "T")); });
    LACKS(html, "skeuo-dialog");
}

TEST(feedback_dialog_open_structure_and_labelling) {
    std::string html = render_static([] {
        return Dialog({.open = true, .aria_describedby = "d"},
                      {
                          DialogTitle({}, "Delete?"),
                          DialogContent({}, DialogContentText({.id = "d"}, "Gone for good.")),
                          DialogActions({}, el("button").child("OK")),
                      });
    });
    for (const char* needle : {"skeuo-dialog skeuo-dialog--max-sm skeuo-dialog--scroll-paper",
                               "skeuo-scrim skeuo-dialog__scrim",
                               "skeuo-chassis skeuo-chassis--float skeuo-elev-4 skeuo-dialog__paper",
                               R"(role="dialog")", R"(aria-modal="true")", R"(aria-describedby="d")",
                               "skeuo-dialog__actions skeuo-dialog__actions--spacing",
                               R"(class="skeuo-dialog__text" id="d")", "data-sk-autofocus"})
        CONTAINS(html, needle);
    // The title's id is the one the dialog is labelled by.
    size_t at = html.find("aria-labelledby=\"");
    CHECK(at != std::string::npos) << html;
    at += 17;
    std::string labelled = html.substr(at, html.find('"', at) - at);
    CHECK(labelled.rfind("skeuo-dialog-title-", 0) == 0) << html;
    CONTAINS(html, R"(<h2 class="skeuo-dialog__title" id=")" + labelled + "\"");
}

TEST(feedback_dialog_options_and_inline_preview) {
    std::string html = render_static([] {
        return Dialog({.open = true,
                       .full_width = true,
                       .max_width = DialogMaxWidth::Xs,
                       .full_screen = true,
                       .scroll = DialogScroll::Body,
                       .inline_ = true,
                       .aria_labelledby = "custom"},
                      {
                          DialogTitle({}, "T"),
                          DialogContent({.dividers = true}, "c"),
                          DialogActions({.disable_spacing = true}, "a"),
                      });
    });
    for (const char* needle : {"skeuo-dialog--max-xs", "skeuo-dialog--scroll-body", "skeuo-dialog--full-width",
                               "skeuo-dialog--full-screen", "skeuo-dialog--inline", "skeuo-chassis--square",
                               R"(aria-modal="false")", R"(aria-labelledby="custom")", R"(id="custom")",
                               "skeuo-dialog__content--dividers"})
        CONTAINS(html, needle);
    LACKS(html, "skeuo-scrim");
    LACKS(html, "skeuo-dialog__actions--spacing");
    LACKS(html, "data-sk-autofocus");
}

TEST(feedback_dialog_title_keeps_its_own_id) {
    std::string html =
        render_static([] { return Dialog({.open = true}, DialogTitle({.id = "mine"}, "T")); });
    CONTAINS(html, R"(<h2 class="skeuo-dialog__title" id="mine">)");
}

TEST(feedback_dialog_escape_scrim_and_paper_clicks) {
    std::vector<ModalCloseReason> reasons;
    bool open = true;
    int action = 0;
    Runtime rt([&] {
        return Dialog({.open = open,
                       .onclose = [&](ModalCloseReason r) { reasons.push_back(r); }},
                      DialogActions({}, Button({.onclick = [&] { ++action; }}, "OK")));
    });
    std::string html = rt.render();
    int esc = feedback_handler(html, "class=\"skeuo-dialog ", "keydown");
    int container = feedback_handler(html, "skeuo-dialog__container", "click");
    int paper = feedback_handler(html, "skeuo-dialog__paper", "click");
    int button = feedback_handler(html, "skeuo-btn ", "click");
    CHECK(esc >= 0 && container >= 0 && paper >= 0 && button >= 0) << html;
    CHECK(html.find("click:" + std::to_string(paper) + "!") != std::string::npos) << "paper stops propagation";
    rt.handle(feedback_event(esc, "keydown", "Escape"));
    rt.handle(feedback_event(container, "click"));
    // A click on a button in the paper reaches the button and the paper,
    // which stops it before the container.
    rt.handle(R"({"t":"event","h":[)" + std::to_string(button) + "," + std::to_string(paper) +
              R"(],"e":{"type":"click"}})");
    CHECK(action == 1);
    CHECK(reasons.size() == 2 && reasons[0] == ModalCloseReason::EscapeKeyDown &&
          reasons[1] == ModalCloseReason::BackdropClick);
}

// ---------------------------------------------------------------- Progress

TEST(feedback_circular_progress_indeterminate) {
    std::string html = render_static([] { return CircularProgress({.aria_label = "Loading"}); });
    for (const char* needle :
         {"skeuo-circular skeuo-circular--indeterminate skeuo-color-primary", R"(role="progressbar")",
          R"(aria-label="Loading")", "width:40px;height:40px;--circular-hole:83.636%",
          "skeuo-groove skeuo-circular__track", "skeuo-circular__arc", R"(r="20.2")", R"(stroke-width="3.6")"})
        CONTAINS(html, needle);
    LACKS(html, "aria-valuenow");
}

TEST(feedback_circular_progress_determinate_value) {
    std::string html = render_static([] {
        return CircularProgress({.variant = CircularProgressVariant::Determinate, .value = 150.0, .size = 60,
                                 .color = Color::Success, .disable_shrink = true});
    });
    for (const char* needle : {"skeuo-circular--determinate skeuo-color-success", R"(aria-valuenow="100")",
                               R"(aria-valuemin="0")", R"(aria-valuemax="100")", "width:60px",
                               "stroke-dashoffset:0"})
        CONTAINS(html, needle);
    // disable_shrink only applies to the indeterminate spinner.
    LACKS(html, "disable-shrink");
    CONTAINS(render_static([] { return CircularProgress({.disable_shrink = true}); }),
             "skeuo-circular--disable-shrink");
}

TEST(feedback_circular_progress_numbers_match_rust) {
    std::string html = render_static([] {
        return Node{
            CircularProgress({.variant = CircularProgressVariant::Determinate, .value = 0, .size = 52, .thickness = 4}),
            CircularProgress({.variant = CircularProgressVariant::Determinate, .value = 66, .size = 28, .thickness = 5}),
        };
    });
    CONTAINS(html, "stroke-dasharray:125.664;stroke-dashoffset:125.664;visibility:hidden");
    CONTAINS(html, "--circular-hole:81.818%");
    CONTAINS(html, "stroke-dasharray:122.522;stroke-dashoffset:41.658");
    CONTAINS(html, R"(aria-valuenow="66")");
}

TEST(feedback_linear_progress_variants) {
    std::string html = render_static([] {
        return LinearProgress({.variant = LinearProgressVariant::Determinate, .value = 42.0, .color = Color::Error});
    });
    for (const char* needle : {"skeuo-groove skeuo-linear skeuo-linear--determinate skeuo-color-error",
                               R"(role="progressbar")", R"(aria-valuenow="42")",
                               "skeuo-groove__fill skeuo-linear__bar", "width:42%"})
        CONTAINS(html, needle);

    html = render_static([] {
        return LinearProgress({.variant = LinearProgressVariant::Buffer, .value = 10.0, .value_buffer = 30.0});
    });
    CONTAINS(html, "skeuo-linear__buffer");
    CONTAINS(html, "width:30%");
    CONTAINS(html, "width:10%");
    CHECK(html.find("width:30%") < html.find("width:10%")) << "buffer comes first";

    html = render_static([] { return LinearProgress({.variant = LinearProgressVariant::Query}); });
    CONTAINS(html, "skeuo-linear--query");
    CONTAINS(html, "skeuo-linear__bar--1");
    CONTAINS(html, "skeuo-linear__bar--2");
    LACKS(html, "aria-valuenow");
}

// ---------------------------------------------------------------- Skeleton

TEST(feedback_skeleton_variants_and_sizes) {
    std::string html = render_static([] { return Skeleton({}); });
    CONTAINS(html,
             "skeuo-well skeuo-well--shallow skeuo-skeleton skeuo-skeleton--text skeuo-skeleton--pulse");
    CONTAINS(html, R"(aria-hidden="true")");
    html = render_static([] {
        return Node{
            Skeleton({.variant = SkeletonVariant::Circular, .animation = SkeletonAnimation::Wave, .width = "40px",
                      .height = "40px"}),
            Skeleton({.variant = SkeletonVariant::Rounded, .animation = SkeletonAnimation::False}, "child"),
        };
    });
    CONTAINS(html, "skeuo-skeleton--circular skeuo-skeleton--wave");
    CONTAINS(html, "width:40px;height:40px;");
    CONTAINS(html, "skeuo-skeleton--rounded skeuo-skeleton--false");
    CONTAINS(html, "child");
}

// ---------------------------------------------------------------- Snackbar

TEST(feedback_snackbar_closed_renders_nothing) {
    LACKS(render_static([] { return Snackbar({.open = false, .message = "Hi"}); }), "skeuo-snackbar");
}

TEST(feedback_snackbar_message_action_anchor_and_timer) {
    std::string html = render_static([] {
        return Snackbar({.open = true,
                         .message = "Saved",
                         .action = el("button").child("Undo"),
                         .auto_hide_duration = 3000,
                         .anchor_origin = {SnackbarVertical::Top, SnackbarHorizontal::Center}});
    });
    for (const char* needle : {"skeuo-snackbar skeuo-snackbar--top skeuo-snackbar--center",
                               "skeuo-chassis skeuo-chassis--float skeuo-elev-3 skeuo-snackbar-content",
                               R"(role="alert")", "Saved", "Undo", "skeuo-snackbar__timer",
                               "--snackbar-hide:3000ms"})
        CONTAINS(html, needle);
}

TEST(feedback_snackbar_defaults_inline_and_children) {
    std::string html = render_static([] { return Snackbar({.open = true, .inline_ = true}, Alert({}, "Custom")); });
    CONTAINS(html, "skeuo-snackbar--bottom skeuo-snackbar--left skeuo-snackbar--inline");
    CONTAINS(html, "skeuo-alert");
    CONTAINS(html, "Custom");
    LACKS(html, "skeuo-snackbar-content");
    LACKS(html, "skeuo-snackbar__timer");

    html = render_static([] { return SnackbarContent({.message = "Plain"}); });
    CONTAINS(html, "skeuo-snackbar-content");
    CONTAINS(html, "Plain");
}

TEST(feedback_snackbar_timeout_and_escape_report_reasons) {
    std::vector<SnackbarCloseReason> reasons;
    Runtime rt([&] {
        return Snackbar({.open = true,
                         .message = "m",
                         .onclose = [&](SnackbarCloseReason r) { reasons.push_back(r); },
                         .auto_hide_duration = 100});
    });
    std::string html = rt.render();
    int timer = feedback_handler(html, "skeuo-snackbar__timer", "animationend");
    int esc = feedback_handler(html, "class=\"skeuo-snackbar ", "keydown");
    CHECK(timer >= 0 && esc >= 0) << html;
    rt.handle(feedback_event(timer, "animationend"));
    rt.handle(feedback_event(esc, "keydown", "Escape"));
    CHECK(reasons.size() == 2 && reasons[0] == SnackbarCloseReason::Timeout &&
          reasons[1] == SnackbarCloseReason::EscapeKeyDown);
    CHECK(as_str(SnackbarCloseReason::Timeout) == "timeout");
}

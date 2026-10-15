#include <map>
#include <memory>

#include "detail.hpp"
#include "skeuo/navigation.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

namespace {

using nav::RovingFocus;

std::string_view variant_name(TabsVariant v) {
    switch (v) {
        case TabsVariant::Standard: return "standard";
        case TabsVariant::Scrollable: return "scrollable";
        case TabsVariant::FullWidth: return "full-width";
    }
    return "standard";
}

std::string_view icon_position_name(IconPosition p) {
    switch (p) {
        case IconPosition::Top: return "top";
        case IconPosition::Bottom: return "bottom";
        case IconPosition::Start: return "start";
        case IconPosition::End: return "end";
    }
    return "top";
}

/// What a Tabs instance shares with the handlers of its tabs (and with an
/// enclosing TabContext, which may override the id and the selection).
struct TabsShared {
    std::optional<std::string> selected;
    std::string id;
    bool controlled = false;
    /// Neither `value` nor an internal value: a TabContext provides it.
    bool free = false;
    bool selection_follows_focus = false;
    std::function<void(const std::string&)> onchange;
    std::optional<std::string>* internal = nullptr;
    std::vector<std::string> values;  // the tabs' values, in order
    std::vector<bool> disabled;

    void select(const std::string& value) {
        if (selected == value) return;
        if (!controlled && internal) *internal = value;
        if (onchange) onchange(value);
    }
};

/// Tabs rendered in the current tree, by id, so a TabContext (which runs
/// after its Tabs) can find their shared state.
std::map<std::string, std::weak_ptr<TabsShared>>& registry() {
    static std::map<std::string, std::weak_ptr<TabsShared>> r;
    return r;
}

void register_tabs(const std::shared_ptr<TabsShared>& st) {
    auto& r = registry();
    for (auto it = r.begin(); it != r.end();) it = it->second.expired() ? r.erase(it) : std::next(it);
    r[st->id] = st;
}

bool is_tab(const Node& n) {
    const std::string* role = n.get_attr("role");
    return n.kind() == Node::Kind::Element && role && *role == "tab" && n.has_class("skeuo-tab");
}

std::vector<Node*> tabs_in(Node& tray) {
    std::vector<Node*> out;
    nav::visit(tray, [&](Node& n) {
        if (is_tab(n)) {
            out.push_back(&n);
            return false;
        }
        return true;
    });
    return out;
}

/// Writes the selection- and id-dependent attributes of every tab.
void restyle(Node& tray, const TabsShared& st) {
    std::vector<Node*> tabs = tabs_in(tray);
    for (size_t i = 0; i < tabs.size() && i < st.values.size(); ++i) {
        Node& tab = *tabs[i];
        const std::string& value = st.values[i];
        bool selected = st.selected == value;
        // Rebuild the class: "skeuo-tab [skeuo-key skeuo-tab--selected] rest".
        std::string cls = "skeuo-tab";
        if (selected) cls += " skeuo-key skeuo-tab--selected";
        for (const auto& t : nav::class_tokens(tab))
            if (t != "skeuo-tab" && t != "skeuo-key" && t != "skeuo-tab--selected") cls += " " + t;
        tab.set_attr("class", cls);
        tab.set_attr("id", st.id + "-tab-" + nav::id_part(value));
        tab.set_attr("aria-selected", selected ? "true" : "false");
        tab.set_attr("aria-controls", st.id + "-panel-" + nav::id_part(value));
        // The selected tab is the tab stop; if none is selected, every tab is.
        tab.set_attr("tabindex", selected || !st.selected ? "0" : "-1");
    }
}

}  // namespace

Node Tab(const TabProps& props, Node children) {
    std::string icon_pos = "skeuo-tab--icon-" + std::string(icon_position_name(props.icon_position));
    Node button = el("button")
                      .cls(classes({"skeuo-tab", props.icon ? std::string_view(icon_pos) : "",
                                    props.wrapped ? "skeuo-tab--wrapped" : "", props.cls}))
                      .attr("type", "button")
                      .attr("role", "tab")
                      .attr("id", "")
                      .attr("aria-selected", "false")
                      .attr("aria-controls", "")
                      .attr("tabindex", "0")
                      .flag("disabled", props.disabled)
                      .attr("data-sk-tab", props.value);
    if (props.onclick) button.on("click", props.onclick);
    if (props.icon)
        button.child(el("span").cls("skeuo-tab__icon").attr("aria-hidden", "true").child(*props.icon));
    if (props.label || !props.icon) {
        Node label = el("span").cls("skeuo-tab__label");
        if (props.label) label.child(*props.label);
        label.child(std::move(children));
        button.child(std::move(label));
    }
    return button;
}

Node Tabs(const TabsProps& props, Node children) {
    std::string id = use_unique_id("skeuo-tabs");
    auto& internal =
        use_state<std::optional<std::string>>("Tabs.internal", [&] { return props.default_value; });
    auto& roving = use_state<RovingFocus>("Tabs.roving", [] { return RovingFocus{}; });

    auto st = std::make_shared<TabsShared>();
    st->selected = props.value ? props.value : internal;
    st->id = id;
    st->controlled = props.value.has_value();
    st->free = !props.value && !internal;
    st->selection_follows_focus = props.selection_follows_focus;
    st->onchange = props.onchange;
    st->internal = &internal;
    register_tabs(st);

    std::string variant = "skeuo-tabs--" + std::string(variant_name(props.variant));
    std::string orientation = "skeuo-tabs--" + std::string(as_str(props.orientation));
    std::string size = "skeuo-tabs--" + std::string(as_str(props.size));

    Node tray = el("div")
                    .cls("skeuo-tabs__tray skeuo-well skeuo-well--shallow")
                    .attr("role", "tablist")
                    .attr("aria-orientation", std::string(as_str(props.orientation)));
    if (props.aria_label) tray.attr("aria-label", *props.aria_label);
    if (props.aria_labelledby) tray.attr("aria-labelledby", *props.aria_labelledby);
    tray.child(std::move(children));

    bool vertical = props.orientation == Orientation::Vertical;
    std::string keys = vertical ? "ArrowUp ArrowDown Home End" : "ArrowLeft ArrowRight Home End";
    std::vector<Node*> tabs = tabs_in(tray);
    for (Node* tab : tabs) {
        st->values.push_back(nav::attr_or(*tab, "data-sk-tab"));
        st->disabled.push_back(nav::has_attr(*tab, "disabled"));
        tab->remove_attr("data-sk-tab");
    }
    for (size_t i = 0; i < tabs.size(); ++i) {
        Node& tab = *tabs[i];
        nav::prepend_listener(tab, "click", [st, i](const Event&) { st->select(st->values[i]); });
        tab.attr("data-sk-keys", keys);
        tab.on("keydown", [st, i, vertical, &roving](const Event& e) {
            std::string next = vertical ? "ArrowDown" : "ArrowRight";
            std::string prev = vertical ? "ArrowUp" : "ArrowLeft";
            std::optional<size_t> target;
            if (e.key == next) target = nav::roving_neighbor(st->disabled, i, 1);
            else if (e.key == prev) target = nav::roving_neighbor(st->disabled, i, -1);
            else if (e.key == "Home") target = nav::roving_neighbor(st->disabled, std::nullopt, 1);
            else if (e.key == "End") target = nav::roving_neighbor(st->disabled, std::nullopt, -1);
            if (!target) return;
            roving.move_to(*target);
            if (st->selection_follows_focus) st->select(st->values[*target]);
        });
    }
    restyle(tray, *st);
    if (auto mark = roving.take_mark(); mark && *mark < tabs.size()) tabs[*mark]->attr("data-sk-autofocus", "");
    nav::freshen(tray, tabs, roving);

    return el("div")
        .cls(classes({"skeuo-tabs", variant, orientation, size, color_class(props.color),
                      props.centered ? "skeuo-tabs--centered" : "", props.cls}))
        .child(std::move(tray));
}

Node TabPanel(const TabPanelProps& props, Node children) {
    Node panel = el("div").cls(classes({"skeuo-tab-panel", props.cls})).attr("role", "tabpanel");
    panel.attr("data-sk-panel", props.value);
    panel.flag("data-sk-keep-mounted", props.keep_mounted);
    return std::move(panel).child(std::move(children));
}

Node TabContext(const TabContextProps& props, Node children) {
    std::string generated = use_unique_id("skeuo-tabs");
    std::string id = props.id.value_or(generated);
    Node out = Node::fragment();
    out.child(std::move(children));
    nav::visit(out, [&](Node& n) {
        if (n.kind() != Node::Kind::Element) return true;
        if (n.has_class("skeuo-tabs__tray")) {
            std::vector<Node*> tabs = tabs_in(n);
            if (tabs.empty()) return false;
            std::string tab_id = nav::attr_or(*tabs.front(), "id");
            for (auto& [key, weak] : registry()) {
                auto st = weak.lock();
                if (!st || tab_id.rfind(key + "-tab-", 0) != 0) continue;
                st->id = id;
                if (st->free) {
                    st->selected = props.value;
                    st->controlled = true;
                }
                restyle(n, *st);
                break;
            }
            return false;
        }
        if (const std::string* value = n.get_attr("data-sk-panel")) {
            bool shown = *value == props.value;
            bool keep = nav::has_attr(n, "data-sk-keep-mounted");
            Node panel = el("div")
                             .attr("class", nav::attr_or(n, "class"))
                             .attr("role", "tabpanel")
                             .attr("id", id + "-panel-" + nav::id_part(*value))
                             .attr("aria-labelledby", id + "-tab-" + nav::id_part(*value))
                             .flag("hidden", !shown)
                             .attr("tabindex", "0");
            if (shown || keep) panel.children(std::move(n.kids()));
            n = std::move(panel);
            return true;
        }
        return true;
    });
    return out;
}

}  // namespace skeuo

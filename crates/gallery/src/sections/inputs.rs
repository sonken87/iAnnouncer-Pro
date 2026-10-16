use dioxus::prelude::*;
use skeuo_ui::prelude::*;

use crate::{Demo, DemoGrid, DemoRow};

#[component]
pub fn InputsSection() -> Element {
    rsx! {
        ButtonDemo {}
        ButtonGroupDemo {}
        IconButtonDemo {}
        FabDemo {}
        ToggleButtonDemo {}
        TextFieldDemo {}
        TextareaAutosizeDemo {}
        SelectDemo {}
        AutocompleteDemo {}
        TransferListDemo {}
    }
}

#[component]
fn ButtonDemo() -> Element {
    let mut clicks = use_signal(|| 0u32);
    let mut saving = use_signal(|| false);
    rsx! {
        Demo {
            title: "Button",
            description: "contained = colored keycap · outlined = white keycap · text = printed legend · loading = spinner legend, key disabled",
            for (variant, label) in [
                (ButtonVariant::Contained, "contained"),
                (ButtonVariant::Outlined, "outlined"),
                (ButtonVariant::Text, "text"),
            ] {
                DemoRow { label,
                    for color in Color::ALL {
                        Button { variant, color, onclick: move |_| clicks += 1, "{color.as_str()}" }
                    }
                }
            }
            DemoRow { label: "sizes",
                Button { size: Size::Small, "Small" }
                Button { size: Size::Medium, "Medium" }
                Button { size: Size::Large, "Large" }
                Button { disabled: true, "Disabled" }
                Button { variant: ButtonVariant::Outlined, disabled: true, "Disabled" }
            }
            DemoRow { label: "icons",
                Button { color: Color::Error, start_icon: rsx! { Icon { name: IconName::Dot } }, "Record" }
                Button { variant: ButtonVariant::Outlined, color: Color::Neutral, end_icon: rsx! { Icon { name: IconName::Play } }, "Play" }
                Button { color: Color::Success, end_icon: rsx! { Icon { name: IconName::ArrowForward } }, "On Air" }
            }
            DemoRow { label: "loading",
                Button { loading: true, "Submit" }
                Button {
                    loading: true,
                    loading_position: LoadingPosition::Start,
                    color: Color::Success,
                    start_icon: rsx! { Icon { name: IconName::Check } },
                    "Saving"
                }
                Button {
                    loading: true,
                    loading_position: LoadingPosition::End,
                    variant: ButtonVariant::Outlined,
                    end_icon: rsx! { Icon { name: IconName::ArrowForward } },
                    "Sending"
                }
                Button { loading: true, variant: ButtonVariant::Text, "Fetch" }
                Button {
                    loading: saving(),
                    loading_position: LoadingPosition::Start,
                    start_icon: rsx! { Icon { name: IconName::Bookmark } },
                    onclick: move |_| saving.set(true),
                    "Save (click)"
                }
                if saving() {
                    Button { variant: ButtonVariant::Text, color: Color::Neutral, onclick: move |_| saving.set(false), "Reset" }
                }
            }
            DemoRow { label: "clicks", span { class: "skeuo-caption", "Clicked {clicks} times" } }
        }
    }
}

#[component]
fn ButtonGroupDemo() -> Element {
    let mut channel = use_signal(|| "A".to_string());
    rsx! {
        Demo {
            title: "ButtonGroup",
            description: "keycaps joined edge-to-edge in one slot, inner corners square · contained / outlined / text · horizontal / vertical",
            DemoRow { label: "contained",
                ButtonGroup { aria_label: "Transport",
                    Button { "One" }
                    Button { "Two" }
                    Button { "Three" }
                }
                ButtonGroup { color: Color::Success,
                    Button { start_icon: rsx! { Icon { name: IconName::Play } }, "Play" }
                    Button { start_icon: rsx! { Icon { name: IconName::Pause } }, "Pause" }
                }
                ButtonGroup { color: Color::Neutral, size: Size::Small,
                    Button { "Small" }
                    Button { "Neutral" }
                    Button { "Group" }
                }
            }
            DemoRow { label: "outlined",
                ButtonGroup { variant: ButtonVariant::Outlined,
                    Button { "One" }
                    Button { "Two" }
                    Button { "Three" }
                }
                ButtonGroup { variant: ButtonVariant::Outlined, color: Color::Error,
                    Button { "Cut" }
                    Button { "Mute" }
                }
                ButtonGroup { variant: ButtonVariant::Outlined, disabled: true,
                    Button { "Disabled" }
                    Button { "Group" }
                }
            }
            DemoRow { label: "text",
                ButtonGroup { variant: ButtonVariant::Text,
                    Button { "One" }
                    Button { "Two" }
                    Button { "Three" }
                }
                ButtonGroup { variant: ButtonVariant::Text, color: Color::Secondary, size: Size::Large,
                    Button { "Large" }
                    Button { "Text" }
                }
            }
            DemoRow { label: "vertical",
                ButtonGroup { orientation: Orientation::Vertical,
                    Button { "Top" }
                    Button { "Middle" }
                    Button { "Bottom" }
                }
                ButtonGroup { orientation: Orientation::Vertical, variant: ButtonVariant::Outlined,
                    Button { "Top" }
                    Button { "Middle" }
                    Button { "Bottom" }
                }
                ButtonGroup { orientation: Orientation::Vertical, variant: ButtonVariant::Text,
                    Button { "Top" }
                    Button { "Middle" }
                    Button { "Bottom" }
                }
                div { style: "flex: 1; min-width: 260px; display: flex; flex-direction: column; gap: 10px",
                    ButtonGroup { full_width: true, variant: ButtonVariant::Outlined, color: Color::Info,
                        for ch in ["A", "B", "C", "D"] {
                            Button { onclick: move |_| channel.set(ch.to_string()), "Ch {ch}" }
                        }
                    }
                    span { class: "skeuo-caption", "full_width · on channel {channel}" }
                }
            }
        }
    }
}

#[component]
fn IconButtonDemo() -> Element {
    let mut liked = use_signal(|| false);
    rsx! {
        Demo {
            title: "IconButton",
            description: "text = icon printed on the aluminium (MUI default) · outlined = small white keycap · contained = colored keycap · round",
            DemoRow { label: "text",
                IconButton { aria_label: "Delete", Icon { name: IconName::Delete } }
                for color in [Color::Primary, Color::Secondary, Color::Success, Color::Error, Color::Warning] {
                    IconButton { color, aria_label: "Favorite", Icon { name: IconName::Favorite } }
                }
                IconButton { size: Size::Small, aria_label: "Settings", Icon { name: IconName::Settings } }
                IconButton { size: Size::Large, aria_label: "Notifications", Icon { name: IconName::Notifications } }
                IconButton { disabled: true, aria_label: "Mail", Icon { name: IconName::Mail } }
            }
            DemoRow { label: "outlined",
                IconButton { variant: IconButtonVariant::Outlined, size: Size::Small, aria_label: "Edit", Icon { name: IconName::Edit } }
                IconButton { variant: IconButtonVariant::Outlined, aria_label: "Edit", Icon { name: IconName::Edit } }
                IconButton { variant: IconButtonVariant::Outlined, size: Size::Large, aria_label: "Edit", Icon { name: IconName::Edit } }
                IconButton { variant: IconButtonVariant::Outlined, color: Color::Primary, aria_label: "Search", Icon { name: IconName::Search } }
                IconButton { variant: IconButtonVariant::Outlined, round: true, color: Color::Error, aria_label: "Delete", Icon { name: IconName::Delete } }
                IconButton { variant: IconButtonVariant::Outlined, disabled: true, aria_label: "Copy", Icon { name: IconName::Copy } }
            }
            DemoRow { label: "contained",
                for color in Color::ALL {
                    IconButton { variant: IconButtonVariant::Contained, color, aria_label: "Add", Icon { name: IconName::Add } }
                }
                IconButton { variant: IconButtonVariant::Contained, color: Color::Error, round: true, size: Size::Large, aria_label: "Record", Icon { name: IconName::Mic } }
            }
            DemoRow { label: "edge",
                div {
                    class: "skeuo-chassis skeuo-elev-1",
                    style: "display: flex; align-items: center; gap: 8px; width: 320px; padding: 6px 16px",
                    IconButton { edge: IconButtonEdge::Start, aria_label: "Menu", Icon { name: IconName::Menu } }
                    span { style: "flex: 1; font-weight: 600", "Edge start / end" }
                    IconButton {
                        edge: IconButtonEdge::End,
                        color: if liked() { Color::Error } else { Color::Neutral },
                        aria_label: "Like",
                        aria_pressed: liked(),
                        onclick: move |_| liked.toggle(),
                        Icon { name: if liked() { IconName::Favorite } else { IconName::FavoriteBorder } }
                    }
                }
                span { class: "skeuo-caption", if liked() { "Liked — click the heart again" } else { "Click the heart" } }
            }
        }
    }
}

#[component]
fn FabDemo() -> Element {
    let mut count = use_signal(|| 0u32);
    rsx! {
        Demo {
            title: "Fab",
            description: "a large round keycap with a deeper travel · circular / extended · small / medium / large",
            DemoRow { label: "circular",
                Fab { color: Color::Primary, aria_label: "Add", onclick: move |_| count += 1, Icon { name: IconName::Add } }
                Fab { color: Color::Secondary, aria_label: "Edit", Icon { name: IconName::Edit } }
                Fab { aria_label: "Like", Icon { name: IconName::Favorite } }
                Fab { color: Color::Error, aria_label: "Record", Icon { name: IconName::Mic } }
                Fab { disabled: true, aria_label: "Navigate", Icon { name: IconName::ArrowForward } }
                span { class: "skeuo-caption", "Added {count}" }
            }
            DemoRow { label: "sizes",
                Fab { size: Size::Small, color: Color::Primary, aria_label: "Add", Icon { name: IconName::Add } }
                Fab { size: Size::Medium, color: Color::Primary, aria_label: "Add", Icon { name: IconName::Add } }
                Fab { size: Size::Large, color: Color::Primary, aria_label: "Add", Icon { name: IconName::Add } }
            }
            DemoRow { label: "extended",
                Fab { variant: FabVariant::Extended, size: Size::Small, color: Color::Primary,
                    Icon { name: IconName::Play }
                    "Small"
                }
                Fab { variant: FabVariant::Extended, size: Size::Medium, color: Color::Success,
                    Icon { name: IconName::Mic }
                    "Go live"
                }
                Fab { variant: FabVariant::Extended,
                    Icon { name: IconName::ArrowForward }
                    "Navigate"
                }
            }
        }
    }
}

#[component]
fn ToggleButtonDemo() -> Element {
    let mut align = use_signal(|| vec!["left".to_string()]);
    let mut formats = use_signal(|| vec!["bold".to_string(), "italic".to_string()]);
    let mut solo = use_signal(|| true);
    let align_text = align
        .read()
        .first()
        .cloned()
        .unwrap_or_else(|| "none".into());
    let formats_text = if formats.read().is_empty() {
        "none".to_string()
    } else {
        formats.read().join(" + ")
    };
    rsx! {
        Demo {
            title: "ToggleButton",
            description: "white keycaps; selected = latched down with the legend lit in color · exclusive / multiple · horizontal / vertical",
            DemoRow { label: "exclusive",
                ToggleButtonGroup {
                    exclusive: true,
                    value: align(),
                    onchange: move |v| align.set(v),
                    aria_label: "Text alignment",
                    ToggleButton { value: "left", aria_label: "Left", Icon { name: IconName::FirstPage } }
                    ToggleButton { value: "center", aria_label: "Center", Icon { name: IconName::Menu } }
                    ToggleButton { value: "right", aria_label: "Right", Icon { name: IconName::LastPage } }
                    ToggleButton { value: "justify", disabled: true, aria_label: "Justify", Icon { name: IconName::MoreHoriz } }
                }
                span { class: "skeuo-caption", "align = {align_text}" }
            }
            DemoRow { label: "multiple",
                ToggleButtonGroup {
                    value: formats(),
                    onchange: move |v| formats.set(v),
                    color: Color::Secondary,
                    aria_label: "Formatting",
                    ToggleButton { value: "bold", b { "B" } }
                    ToggleButton { value: "italic", i { "I" } }
                    ToggleButton { value: "underline", u { "U" } }
                    ToggleButton { value: "color", "Color" }
                }
                span { class: "skeuo-caption", "formats = {formats_text}" }
            }
            DemoRow { label: "sizes",
                ToggleButtonGroup { size: Size::Small, exclusive: true, default_value: vec!["web".to_string()],
                    ToggleButton { value: "web", "Web" }
                    ToggleButton { value: "android", "Android" }
                    ToggleButton { value: "ios", "iOS" }
                }
                ToggleButtonGroup { exclusive: true, default_value: vec!["android".to_string()], color: Color::Success,
                    ToggleButton { value: "web", "Web" }
                    ToggleButton { value: "android", "Android" }
                    ToggleButton { value: "ios", "iOS" }
                }
                ToggleButtonGroup { size: Size::Large, exclusive: true, default_value: vec!["ios".to_string()], color: Color::Error,
                    ToggleButton { value: "web", "Web" }
                    ToggleButton { value: "android", "Android" }
                    ToggleButton { value: "ios", "iOS" }
                }
            }
            DemoRow { label: "vertical",
                ToggleButtonGroup {
                    orientation: Orientation::Vertical,
                    exclusive: true,
                    default_value: vec!["list".to_string()],
                    ToggleButton { value: "list", aria_label: "List", Icon { name: IconName::Menu } }
                    ToggleButton { value: "module", aria_label: "Module", Icon { name: IconName::Copy } }
                    ToggleButton { value: "quilt", aria_label: "Quilt", Icon { name: IconName::Folder } }
                }
                ToggleButton {
                    value: "check",
                    selected: solo(),
                    color: Color::Success,
                    onchange: move |_| solo.toggle(),
                    aria_label: "Standalone",
                    Icon { name: IconName::Check }
                }
                ToggleButton { value: "star", aria_label: "Star", Icon { name: IconName::Star } }
                span { class: "skeuo-caption", "standalone: {solo}" }
                div { style: "flex: 1; min-width: 260px",
                    ToggleButtonGroup { full_width: true, exclusive: true, default_value: vec!["day".to_string()],
                        ToggleButton { value: "day", "Day" }
                        ToggleButton { value: "week", "Week" }
                        ToggleButton { value: "month", "Month" }
                    }
                }
            }
        }
    }
}

#[component]
fn TextFieldDemo() -> Element {
    let mut name = use_signal(String::new);
    let mut email = use_signal(|| "operator@station".to_string());
    let email_invalid = !email.read().contains('@');
    rsx! {
        Demo {
            title: "TextField",
            description: "outlined = well · filled = shallow recess · standard = engraved line",
            DemoGrid {
                TextField {
                    label: "Announcer",
                    placeholder: "Your name",
                    value: name(),
                    oninput: move |e: FormEvent| name.set(e.value()),
                    helper_text: if name.read().is_empty() { "Required".to_string() } else { format!("Hello, {}", name.read()) },
                    required: true,
                }
                TextField {
                    label: "Email",
                    r#type: "email",
                    value: email(),
                    oninput: move |e: FormEvent| email.set(e.value()),
                    error: email_invalid,
                    helper_text: if email_invalid { "Must contain @" } else { "Looks good" },
                }
                TextField { label: "Password", r#type: "password", default_value: "secret" }
                TextField { label: "Filled", variant: TextFieldVariant::Filled, placeholder: "Shallow recess" }
                TextField {
                    label: "Frequency",
                    variant: TextFieldVariant::Filled,
                    default_value: "101.7",
                    end_adornment: rsx! { "MHz" },
                }
                TextField { label: "Disabled", default_value: "Read only", disabled: true }
                TextField { label: "Standard", variant: TextFieldVariant::Standard, placeholder: "Engraved line" }
                TextField {
                    label: "Search",
                    variant: TextFieldVariant::Standard,
                    start_adornment: rsx! { Icon { name: IconName::Search } },
                    placeholder: "Find a clip",
                }
                TextField { label: "Small", size: Size::Small, placeholder: "size=small" }
            }
            div { style: "margin-top: 20px",
                TextField {
                    label: "Script",
                    multiline: true,
                    rows: 4,
                    full_width: true,
                    placeholder: "Type the announcement…",
                }
            }
        }
    }
}

#[component]
fn TextareaAutosizeDemo() -> Element {
    let mut notes = use_signal(|| "Line one\nLine two".to_string());
    let lines = notes.read().split('\n').count();
    rsx! {
        Demo {
            title: "TextareaAutosize",
            description: "the TextField well, growing with its content between min_rows and max_rows",
            DemoGrid {
                div { style: "display: flex; flex-direction: column; gap: 6px",
                    span { class: "skeuo-label", "Empty, min_rows = 2" }
                    TextareaAutosize { min_rows: 2, placeholder: "Grows as you type…", aria_label: "Notes" }
                }
                div { style: "display: flex; flex-direction: column; gap: 6px",
                    span { class: "skeuo-label", "Controlled, max_rows = 4" }
                    TextareaAutosize {
                        max_rows: 4,
                        value: notes(),
                        oninput: move |e: FormEvent| notes.set(e.value()),
                        aria_label: "Controlled notes",
                    }
                    span { class: "skeuo-caption", "{lines} line(s)" }
                }
                div { style: "display: flex; flex-direction: column; gap: 6px",
                    span { class: "skeuo-label", "Capped at max_rows = 3" }
                    TextareaAutosize {
                        max_rows: 3,
                        default_value: "Station ID\nTraffic\nWeather\nNews at the top of the hour\nMusic",
                        aria_label: "Running order",
                    }
                }
                div { style: "display: flex; flex-direction: column; gap: 6px",
                    span { class: "skeuo-label", "Disabled" }
                    TextareaAutosize { disabled: true, default_value: "Locked", aria_label: "Locked" }
                }
            }
        }
    }
}

fn stations() -> Vec<SelectOption> {
    vec![
        SelectOption::new("fm1017", "101.7 FM"),
        SelectOption::new("fm954", "95.4 FM"),
        SelectOption::new("am810", "810 AM"),
        SelectOption::new("dab", "DAB+ Digital"),
        SelectOption::new("off", "Off air").disabled(),
    ]
}

#[component]
fn SelectDemo() -> Element {
    let mut station = use_signal(|| "fm954".to_string());
    let mut days = use_signal(|| vec!["mon".to_string(), "wed".to_string()]);
    let mut native = use_signal(|| "15".to_string());
    let days_options: Vec<SelectOption> = [
        ("mon", "Monday"),
        ("tue", "Tuesday"),
        ("wed", "Wednesday"),
        ("thu", "Thursday"),
        ("fri", "Friday"),
    ]
    .into_iter()
    .map(|(v, l)| SelectOption::new(v, l))
    .collect();
    let minutes: Vec<SelectOption> = ["5", "10", "15", "30", "60"]
        .into_iter()
        .map(|m| SelectOption::new(m, format!("{m} minutes")))
        .collect();
    rsx! {
        Demo {
            title: "Select",
            description: "a well with a chevron; the open list is a floating plate (chassis 3) with tinted, ticked rows · options passed as data",
            DemoGrid {
                Select {
                    label: "Station",
                    options: stations(),
                    value: station(),
                    onchange: move |v| station.set(v),
                    helper_text: format!("value = {station}"),
                }
                Select {
                    label: "Days",
                    multiple: true,
                    options: days_options.clone(),
                    values: days(),
                    onchange_multiple: move |v| days.set(v),
                    helper_text: "multiple",
                }
                Select { label: "Placeholder", options: stations(), placeholder: "Choose a station" }
                Select { label: "Filled", variant: TextFieldVariant::Filled, options: stations(), default_value: "am810" }
                Select { label: "Standard", variant: TextFieldVariant::Standard, options: stations(), default_value: "dab" }
                Select { label: "Small", size: Size::Small, options: stations(), default_value: "fm1017" }
                Select {
                    label: "Error",
                    options: stations(),
                    error: true,
                    required: true,
                    placeholder: "Pick one",
                    helper_text: "A station is required",
                }
                Select { label: "Disabled", options: stations(), default_value: "fm1017", disabled: true }
                NativeSelect {
                    label: "NativeSelect",
                    options: minutes,
                    value: native(),
                    onchange: move |v| native.set(v),
                    helper_text: format!("platform list · {native} min"),
                }
            }
            div { style: "margin-top: 20px; min-height: 252px",
                DemoGrid {
                    Select {
                        label: "Open (static preview)",
                        options: stations(),
                        default_value: "fm954",
                        open: true,
                    }
                    Select {
                        label: "Open, multiple",
                        multiple: true,
                        color: Color::Secondary,
                        options: days_options,
                        default_values: vec!["tue".to_string(), "thu".to_string()],
                        open: true,
                    }
                }
            }
        }
    }
}

fn programmes() -> Vec<String> {
    [
        "Breakfast Show",
        "Drive Time",
        "Late Night Jazz",
        "Morning News",
        "Sports Roundup",
        "The Chart",
        "Weather Watch",
        "Weekend Brunch",
    ]
    .into_iter()
    .map(String::from)
    .collect()
}

#[component]
fn AutocompleteDemo() -> Element {
    let mut pick = use_signal(|| Some("Drive Time".to_string()));
    let mut tags = use_signal(|| vec!["Morning News".to_string(), "The Chart".to_string()]);
    let pick_text = pick().unwrap_or_else(|| "none".into());
    rsx! {
        Demo {
            title: "Autocomplete",
            description: "a well that filters options as you type; multiple values become keycap pills inside the well · free_solo accepts any text",
            DemoGrid {
                Autocomplete {
                    label: "Programme",
                    options: programmes(),
                    value: pick(),
                    onchange: move |v| pick.set(v),
                    placeholder: "Type to filter",
                    helper_text: format!("value = {pick_text}"),
                }
                Autocomplete {
                    label: "Free solo",
                    options: programmes(),
                    free_solo: true,
                    placeholder: "Any title",
                    helper_text: "Press Enter to keep your own text",
                }
                Autocomplete { label: "Disabled", options: programmes(), default_value: "The Chart", disabled: true }
            }
            div { style: "margin-top: 20px",
                Autocomplete {
                    label: "Playlist (multiple)",
                    options: programmes(),
                    multiple: true,
                    values: tags(),
                    onchange_multiple: move |v| tags.set(v),
                    placeholder: "Add programmes",
                    full_width: true,
                    helper_text: format!("{} selected · Backspace removes the last pill", tags.read().len()),
                }
            }
            div { style: "margin-top: 20px; min-height: 196px",
                DemoGrid {
                    Autocomplete {
                        label: "Open (static preview)",
                        options: programmes(),
                        default_values: vec!["Late Night Jazz".to_string()],
                        multiple: true,
                        input_value: "n",
                        open: true,
                    }
                    Autocomplete {
                        label: "No match",
                        options: programmes(),
                        input_value: "xyz",
                        open: true,
                        no_options_text: "No programmes found",
                    }
                }
            }
        }
    }
}

#[component]
fn TransferListDemo() -> Element {
    let mut lists = use_signal(|| {
        (
            vec![
                "Jingle A".to_string(),
                "Jingle B".to_string(),
                "Station ID".to_string(),
                "Traffic".to_string(),
                "Weather".to_string(),
            ],
            vec!["Top of hour".to_string(), "News".to_string()],
        )
    });
    let (left, right) = lists();
    rsx! {
        Demo {
            title: "TransferList",
            description: "two wells of rows with small checkbox keycaps; keycaps between them move the checked rows across",
            TransferList {
                left: left.clone(),
                right: right.clone(),
                default_checked: vec!["Station ID".to_string(), "Weather".to_string()],
                onchange: move |v| lists.set(v),
                left_title: "Library",
                right_title: "Running order",
            }
            p { class: "skeuo-caption", style: "margin: 12px 0 0",
                "Running order: {right.join(\" → \")}"
            }
        }
    }
}

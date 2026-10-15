//! One page per component category. Each page is a list of `Demo` plates.

mod data_display;
mod feedback;
mod icons;
mod inputs;
mod navigation;
mod selection;
mod surfaces;

pub use data_display::DataDisplaySection;
pub use feedback::FeedbackSection;
pub use icons::IconsSection;
pub use inputs::InputsSection;
pub use navigation::NavigationSection;
pub use selection::SelectionSection;
pub use surfaces::SurfacesSection;

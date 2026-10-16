// Component gallery for skeuo-ui (C++). One page per component category,
// mirroring crates/gallery in the Rust version.
#pragma once

#include <string>
#include <string_view>

#include "skeuo/skeuo.hpp"

namespace gallery {

using namespace skeuo;

enum class Page { Inputs, Selection, DataDisplay, Feedback, Surfaces, Navigation, Map, Icons };

inline constexpr Page kAllPages[] = {Page::Inputs,   Page::Selection,  Page::DataDisplay, Page::Feedback,
                                     Page::Surfaces, Page::Navigation, Page::Map,         Page::Icons};

std::string_view page_title(Page p);
std::string_view page_slug(Page p);

/// One demo plate: a titled aluminium panel holding examples of a component.
Node Demo(std::string title, std::string description, Node children);
/// A labelled row of examples inside a Demo (pass "" for no label).
Node DemoRow(std::string label, Node children);
/// A responsive grid of examples inside a Demo.
Node DemoGrid(Node children);

// Pages (gallery/pages/*.cpp).
Node InputsPage();
Node SelectionPage();
Node DataDisplayPage();
Node FeedbackPage();
Node SurfacesPage();
Node NavigationPage();
Node MapPage();
Node IconsPage();

}  // namespace gallery

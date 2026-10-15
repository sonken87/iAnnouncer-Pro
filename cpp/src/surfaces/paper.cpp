#include "skeuo/surfaces.hpp"

namespace skeuo {

std::string chassis_classes(PaperVariant variant, int elevation, bool square) {
    int level = variant == PaperVariant::Elevation ? chassis_level(elevation) : 0;
    std::string elev = "skeuo-elev-" + std::to_string(level);
    std::string variant_class = "skeuo-paper--" + std::string(as_str(variant));
    return classes({"skeuo-chassis", elev, level >= 3 ? "skeuo-chassis--float" : "",
                    square ? "skeuo-chassis--square" : "", "skeuo-paper", variant_class});
}

Node Paper(const PaperProps& props, Node children) {
    Node div = el("div").cls(classes({chassis_classes(props.variant, props.elevation, props.square), props.cls}));
    for (const auto& [k, v] : props.attrs) div.set_attr(k, v);
    return std::move(div).child(std::move(children));
}

}  // namespace skeuo

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ngraph/ngraph.hpp"
#include "ngraph/op/util/attr_types.hpp"
#include "ngraph/pass/manager.hpp"
#include "ngraph/pass/op_downgrade_v1tov0.hpp"
#include "ngraph/pass/op_upgrade_v0tov1.hpp"
#include "util/type_prop.hpp"

using namespace std;
using namespace ngraph;

TEST(opset_transform, opset1_broadcast_upgrade_pass)
{
    auto arg = make_shared<op::Parameter>(element::f32, Shape{5, 6});

    auto bcast_v0 = make_shared<op::v0::Broadcast>(arg, Shape{3, 5, 4, 6}, AxisSet{0, 2});
    auto f = make_shared<Function>(NodeVector{bcast_v0}, ParameterVector{arg});

    ngraph::pass::Manager pass_manager;
    pass_manager.register_pass<pass::OpUpgradeV0ToV1>();
    pass_manager.run_passes(f);

    auto bcast_v1 = static_pointer_cast<op::v1::Broadcast>(
        f->get_results().at(0)->input_value(0).get_node_shared_ptr());

    EXPECT_EQ(bcast_v1->description(), "Broadcast");
    EXPECT_EQ(bcast_v1->get_version(), 1);
    EXPECT_EQ(bcast_v1->get_broadcast_spec(), op::AutoBroadcastSpec());
    EXPECT_EQ(bcast_v1->get_broadcast_axes(), (std::make_pair<bool, AxisSet>(true, AxisSet{0, 2})));

    EXPECT_EQ(bcast_v1->input_value(1).get_node()->description(), "Constant");
    EXPECT_EQ(bcast_v1->input_value(2).get_node()->description(), "Constant");
    EXPECT_EQ(static_pointer_cast<op::Constant>(bcast_v1->input_value(1).get_node_shared_ptr())
                  ->get_shape_val(),
              (Shape{3, 5, 4, 6}));
    EXPECT_EQ(static_pointer_cast<op::Constant>(bcast_v1->input_value(2).get_node_shared_ptr())
                  ->get_axis_set_val(),
              (AxisSet{1, 3}));
}

TEST(opset_transform, opset1_broadcast_downgrade_pass)
{
    auto arg = make_shared<op::Parameter>(element::f32, Shape{1, 2, 3});
    auto target_shape = op::Constant::create<int64_t>(element::i64, Shape{5}, {3, 1, 4, 2, 3});
    auto axes_mapping = op::Constant::create<int64_t>(element::i64, Shape{3}, {1, 3, 4});

    auto bcast_v1 = make_shared<op::v1::Broadcast>(arg, target_shape, axes_mapping);
    auto f = make_shared<Function>(NodeVector{bcast_v1}, ParameterVector{arg});

    ngraph::pass::Manager pass_manager;
    pass_manager.register_pass<pass::OpDowngradeV1ToV0>();
    pass_manager.run_passes(f);

    auto bcast_v0 = static_pointer_cast<op::v0::Broadcast>(
        f->get_results().at(0)->input_value(0).get_node_shared_ptr());

    EXPECT_EQ(bcast_v0->description(), "Broadcast");
    EXPECT_EQ(bcast_v0->get_version(), 0);
    EXPECT_EQ(bcast_v0->get_broadcast_shape(), (Shape{3, 1, 4, 2, 3}));
    EXPECT_EQ(bcast_v0->get_broadcast_axes(), (AxisSet{0, 2}));
}

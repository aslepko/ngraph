//*****************************************************************************
// Copyright 2017-2019 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#include "ngraph/op/acos.hpp"

#include "ngraph/axis_set.hpp"
#include "ngraph/op/broadcast.hpp"
#include "ngraph/op/constant.hpp"
#include "ngraph/op/divide.hpp"
#include "ngraph/op/multiply.hpp"
#include "ngraph/op/negative.hpp"
#include "ngraph/op/sqrt.hpp"
#include "ngraph/op/subtract.hpp"
#include "ngraph/shape.hpp"

#include <string>
#include <vector>

using namespace std;
using namespace ngraph;

op::Acos::Acos(const NodeOutput& arg)
    : UnaryElementwiseArithmetic("Acos", arg)
{
    constructor_validate_and_infer_types();
}

shared_ptr<Node>
    op::Acos::copy_with_new_source_outputs(const OutputVector& new_source_outputs) const
{
    check_new_source_outputs_count(this, new_source_outputs);
    return make_shared<Acos>(new_source_outputs.at(0));
}

void op::Acos::generate_adjoints(autodiff::Adjoints& adjoints, const NodeVector& deltas)
{
    auto delta = deltas.at(0);

    auto x = get_argument(0);

    auto one = make_shared<op::ScalarConstantLike>(x, 1.0);
    auto ones = make_shared<op::BroadcastLike>(one, x, AxisSet());

    adjoints.add_delta(x, -delta / make_shared<op::Sqrt>(ones - x * x));
}

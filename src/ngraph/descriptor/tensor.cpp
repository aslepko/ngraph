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

#include "ngraph/descriptor/tensor.hpp"
#include "ngraph/descriptor/layout/tensor_layout.hpp"
#include "ngraph/node.hpp"

using namespace ngraph;
using namespace std;

descriptor::Tensor::Tensor(const element::Type& element_type,
                           const PartialShape& pshape,
                           const std::string& name)
    : m_element_type(element_type)
    , m_shape(pshape.is_static() ? pshape.to_shape() : Shape{})
    , m_partial_shape(pshape)
    , m_name(name)
    , m_is_valid(true)
    , m_invalidity_explanation("")
{
}

descriptor::Tensor::Tensor(const element::Type& element_type,
                           const PartialShape& pshape,
                           Node* node,
                           size_t node_output_number)
    : m_element_type(element_type)
    , m_shape(pshape.is_static() ? pshape.to_shape() : Shape{})
    , m_partial_shape(pshape)
    , m_node(node)
    , m_node_output_number(node_output_number)
    , m_is_valid(true)
    , m_invalidity_explanation("")
{
}

void descriptor::Tensor::set_tensor_type(const element::Type& element_type,
                                         const PartialShape& pshape,
                                         bool is_valid,
                                         const std::string& invalidity_explanation)
{
    if (is_valid)
    {
        NGRAPH_CHECK(pshape.all_non_negative(),
                     "set_tensor_type called on a PartialShape containing negative dimensions: ",
                     pshape);
        if (pshape.is_static())
        {
            m_shape = pshape.to_shape();
        }
        else
        {
            m_shape = Shape{};
        }
        m_partial_shape = pshape;
        m_element_type = element_type;
    }
    else
    {
        m_partial_shape = PartialShape::dynamic();
        m_element_type = element::dynamic;
    }

    m_is_valid = is_valid;
    m_invalidity_explanation = "";
}

const element::Type& descriptor::Tensor::get_element_type() const
{
    NGRAPH_CHECK(m_is_valid,
                 "get_element_type() called on an invalid node. Original validation error: ",
                 m_invalidity_explanation);
    return m_element_type;
}

const Shape& descriptor::Tensor::get_shape() const
{
    NGRAPH_CHECK(m_is_valid,
                 "get_shape() called on an invalid node. Original validation error: ",
                 m_invalidity_explanation);
    if (m_partial_shape.is_static())
    {
        return m_shape;
    }
    else
    {
        throw std::invalid_argument(
            "get_shape was called on a descriptor::Tensor with dynamic shape");
    }
}

const PartialShape& descriptor::Tensor::get_partial_shape() const
{
    NGRAPH_CHECK(m_is_valid,
                 "get_partial_shape() called on an invalid node. Original validation error: ",
                 m_invalidity_explanation);
    return m_partial_shape;
}

void descriptor::Tensor::set_pool_offset(size_t offset)
{
    m_pool_offset = offset;
}

size_t descriptor::Tensor::get_pool_offset() const
{
    return m_pool_offset;
}

size_t descriptor::Tensor::size() const
{
    if (auto tvl = get_tensor_layout())
    {
        return tvl->get_allocated_size();
    }
    else
    {
        return shape_size(get_shape()) * m_element_type.size();
    }
}

bool descriptor::Tensor::is_valid() const
{
    return m_is_valid;
}

const std::string& descriptor::Tensor::get_invalidity_explanation() const
{
    return m_invalidity_explanation;
}

void descriptor::Tensor::set_tensor_layout(
    const std::shared_ptr<layout::TensorLayout>& tensor_layout)
{
    if (tensor_layout->get_shape() != get_shape())
    {
        throw ngraph_error("Setting tensor's layout to a layout with a different shape.");
    }
    if (tensor_layout->get_element_type() != get_element_type())
    {
        throw ngraph_error("Setting tensor's layout to a layout with a different element type.");
    }
    m_tensor_layout = tensor_layout;
}

const std::string& descriptor::Tensor::get_name() const
{
    if (m_name.empty() && m_node != nullptr)
    {
        const_cast<Tensor*>(this)->m_name =
            m_node->get_name() + "_" + to_string(m_node_output_number);
    }
    return m_name;
}

ostream& operator<<(ostream& out, const descriptor::Tensor& tensor)
{
    out << "Tensor(" << tensor.get_name() << ")";
    return out;
}

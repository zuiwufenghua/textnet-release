#ifndef TEXTNET_LAYER_IMPL_INL_HPP_
#define TEXTNET_LAYER_IMPL_INL_HPP_

#include "./layer.h"
#include "./common/activation_layer-inl.hpp"
#include "./common/convolution_layer-inl.hpp"
#include "./common/fullc_layer-inl.hpp"
#include "./common/pooling_layer-inl.hpp"
#include "./common/embedding_layer-inl.hpp"
#include "./common/cross_layer-inl.hpp"
#include "./common/split_layer-inl.hpp"
#include "./common/dropout_layer-inl.hpp"
#include "./input/textdata_layer-inl.hpp"
#include "./loss/hingeloss_layer-inl.hpp"
#include "./loss/pairhingeloss_layer-inl.hpp"
#include "./loss/softmax_layer-inl.hpp"

namespace textnet {
namespace layer {
template<typename xpu>
Layer<xpu>* CreateLayer_(LayerType type) {
  switch(type) {
    case kSigmoid: return new ActivationLayer<xpu, op::sigmoid, op::sigmoid_grad>(type);
    case kTanh: return new ActivationLayer<xpu, op::tanh, op::tanh_grad>(type);
    case kRectifiedLinear: return new ActivationLayer<xpu, op::relu, op::relu_grad>(type);
    case kConv: return new ConvolutionLayer<xpu>(type);
    case kFullConnect: return new FullConnectLayer<xpu>(type);
    case kMaxPooling: return new PoolingLayer<mshadow::red::maximum, xpu>(kMaxPooling);
    case kAvgPooling: return new PoolingLayer<mshadow::red::sum, xpu>(kAvgPooling);
    case kEmbedding: return new EmbeddingLayer<xpu>(kEmbedding);
    case kCross: return new CrossLayer<xpu>(kCross);
    case kSplit: return new SplitLayer<xpu>(kSplit);
    case kDropout: return new DropoutLayer<xpu>(kDropout);
    case kHingeLoss: return new HingeLossLayer<xpu>(kHingeLoss);
    case kPairHingeLoss: return new PairHingeLossLayer<xpu>(kPairHingeLoss);
    case kTextData: return new TextDataLayer<xpu>(kTextData);
    case kSoftmax: return new SoftmaxLayer<xpu>(kSoftmax);
    default: utils::Error("unknown layer type id : \"%d\"", type); return NULL;
  }
}

}  // namespace layer
}  // namespace textnet
#endif

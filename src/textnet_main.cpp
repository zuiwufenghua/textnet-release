#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>
#include <ctime>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <climits>

#include "./layer/layer.h"
#include "global.h"

using namespace std;
using namespace textnet;
using namespace textnet::layer;
using namespace mshadow;

void PrintTensor(Tensor<cpu, 3> x) {
	Shape<3> s = x.shape_;
	for (unsigned int d1 = 0; d1 < s[0]; ++d1) {
		for (unsigned int d2 = 0; d2 < s[1]; ++d2) {
			for (unsigned int d3 = 0; d3 < s[2]; ++d3) {
					cout << x[d1][d2][d3] << " ";
			}
			cout << ";";
		}
		cout << endl;
    }
}


void PrintTensor(const char * name, Tensor<cpu, 4> x) {
	Shape<4> s = x.shape_;
	cout << name << " shape " << s[0] << "x" << s[1] << "x" << s[2] << "x" << s[3] << endl;
	for (unsigned int d1 = 0; d1 < s[0]; ++d1) {
		for (unsigned int d2 = 0; d2 < s[1]; ++d2) {
			for (unsigned int d3 = 0; d3 < s[2]; ++d3) {
				for (unsigned int d4 = 0; d4 < s[3]; ++d4) {
					cout << x[d1][d2][d3][d4] << " ";
				}
				cout << "|";
			}
			cout << ";";
		}
		cout << endl;
    }
	cout << endl;
}

void PrintTensorP(Tensor<cpu, 4> x) {
	Shape<4> s = x.shape_;
	float * p = x.dptr_;
	for (int i = 0; i < x.shape_.Size(); ++i) {
		cout << *p << " ";
		p++;
	}
}

void TestCrossLayer(mshadow::Random<cpu>* prnd) {
  Node<cpu> bottom1;
  Node<cpu> bottom2;
  Node<cpu> top;
  vector<Node<cpu>*> bottoms;
  vector<Node<cpu>*> tops;

  bottoms.push_back(&bottom1);
  bottoms.push_back(&bottom2);
  tops.push_back(&top);

  bottom1.Resize(2, 1, 1, 5);
  bottom2.Resize(2, 1, 1, 5);
  bottom1.data = 1.0;
  bottom2.data = 2.0;

  map<string, SettingV> setting;
  // Test Cross Layer
  Layer<cpu> * layer_cross = CreateLayer<cpu>(kCross);
  layer_cross->PropAll();
  layer_cross->SetupLayer(setting, bottoms, tops, prnd);
  layer_cross->Reshape(bottoms, tops);
  layer_cross->Forward(bottoms, tops);
  top.diff = 1.0;
  top.diff[0][0][2][3] = 2.0;
  layer_cross->Backprop(bottoms, tops);
  
  PrintTensor("top", top.data);
  PrintTensor("bottom1 diff", bottom1.diff);
  PrintTensor("bottom2 diff", bottom2.diff);
}

void TestConvLayer(mshadow::Random<cpu>* prnd) {

}

void TestPoolLayer(mshadow::Random<cpu>* prnd) {

}

void TestTextDataLayer(mshadow::Random<cpu>* prnd) {
  Node<cpu> top1;
  Node<cpu> top2;
  vector<Node<cpu>*> bottoms;
  vector<Node<cpu>*> tops;
  
  tops.push_back(&top1);
  tops.push_back(&top2);

  map<string, SettingV> setting;
  setting["data_file"] = SettingV("/home/pangliang/matching/data/msr_paraphrase_train_wid.txt");
  setting["batch_size"] = SettingV(2);
  setting["max_doc_len"] = SettingV(31);
  setting["min_doc_len"] = SettingV(5);
  
  /// Test TextData Layer
  Layer<cpu> * layer_textdata = CreateLayer<cpu>(kTextData);
  layer_textdata->PropAll();
  layer_textdata->SetupLayer(setting, bottoms, tops, prnd);
  layer_textdata->Reshape(bottoms, tops);
  layer_textdata->Forward(bottoms, tops);
  layer_textdata->Backprop(bottoms, tops);

  PrintTensor("top1", top1.data);
  PrintTensor("top2", top2.data);

  vector<Node<cpu>*> bottoms_wv;
  vector<Node<cpu>*> tops_wv;
  Node<cpu> top_wv;

  bottoms_wv.push_back(&top1);
  tops_wv.push_back(&top_wv);

  map<string, SettingV> setting_wv;
  map<string, SettingV> setting_wfiller;
  setting_wv["embedding_file"] = SettingV("/home/pangliang/matching/data/wikicorp_50_msr.txt");
  setting_wv["word_count"] = SettingV(14727);
  setting_wv["feat_size"] = SettingV(50);
  setting_wfiller["init_type"] = SettingV(initializer::kGaussian);
  setting_wfiller["mu"] = SettingV(0.0f);
  setting_wfiller["sigma"] = SettingV(1.0f);
  setting_wv["w_filler"] = SettingV(&setting_wfiller);

  // Test Embedding Layer
  Layer<cpu> * layer_embedding = CreateLayer<cpu>(kEmbedding);
  layer_embedding->PropAll();
  layer_embedding->SetupLayer(setting_wv, bottoms_wv, tops_wv, prnd);
  layer_embedding->Reshape(bottoms_wv, tops_wv);
  layer_embedding->Forward(bottoms_wv, tops_wv);
  top_wv.diff = 1.0;
  layer_embedding->Backprop(bottoms_wv, tops_wv);

  PrintTensor("top_wv", top_wv.data);
  PrintTensor("top1_diff", top1.diff);
  PrintTensor("weight_diff", layer_embedding->GetParams()[0].diff);
  PrintTensor("weight_idx", layer_embedding->GetParams()[0].idx);

  vector<Node<cpu>*> bottoms_sp;
  vector<Node<cpu>*> tops_sp;
  Node<cpu> top1_sp;
  Node<cpu> top2_sp;

  bottoms_sp.push_back(&top_wv);
  tops_sp.push_back(&top1_sp);
  tops_sp.push_back(&top2_sp);

  // Test Split Layer
  Layer<cpu> * layer_split = CreateLayer<cpu>(kSplit);
  layer_split->PropAll();
  layer_split->SetupLayer(setting, bottoms_sp, tops_sp, prnd);
  layer_split->Reshape(bottoms_sp, tops_sp);
  layer_split->Forward(bottoms_sp, tops_sp);
  top1_sp.diff = 1.0;
  top2_sp.diff = 2.0;
  layer_split->Backprop(bottoms_sp, tops_sp);

  PrintTensor("top1_sp", top1_sp.data);
  PrintTensor("top2_sp", top2_sp.data);
  PrintTensor("top_wv_diff", top_wv.diff);
}

void TestActivationLayer(mshadow::Random<cpu>* prnd) {
  Node<cpu> bottom;
  Node<cpu> top;
  vector<Node<cpu>*> bottoms;
  vector<Node<cpu>*> tops;
  
  bottoms.push_back(&bottom);
  tops.push_back(&top);
  
  bottom.data.Resize(Shape4(2,1,5,5), 1.0);
  bottom.diff.Resize(Shape4(2,1,5,5), 0.0);
  
  map<string, SettingV> setting;
  //setting["layer_type"] = SettingV(kRectifiedLinear);
  
  /// Test Activation Layer
  Layer<cpu> * layer_rectify = CreateLayer<cpu>(kRectifiedLinear);
  layer_rectify->PropAll();
  layer_rectify->SetupLayer(setting, bottoms, tops, prnd);
  layer_rectify->Reshape(bottoms, tops);
  layer_rectify->Forward(bottoms, tops);
  top.diff.Resize(Shape4(2,1,5,5), 1.0);
  layer_rectify->Backprop(bottoms, tops);

  cout << "bottom size: " << bottom.data.shape_[0] << "x" << bottom.data.shape_[1] << "x" << bottom.data.shape_[2] << "x" << bottom.data.shape_[3] << endl;
  cout << "top size: " << top.data.shape_[0] << "x" << top.data.shape_[1] << "x" << top.data.shape_[2] << "x" << top.data.shape_[3] << endl;
  
  cout << "top data: " << top.data[0][0][0][0] << endl;
  cout << "bottom diff: " << bottom.diff[0][0][0][0] << endl;
  
  Layer<cpu> * layer_sigmoid = CreateLayer<cpu>(kSigmoid);
  layer_sigmoid->PropAll();
  layer_sigmoid->SetupLayer(setting, bottoms, tops, prnd);
  layer_sigmoid->Reshape(bottoms, tops);
  layer_sigmoid->Forward(bottoms, tops);
  top.diff.Resize(Shape4(2,1,5,5), 1.0);
  layer_sigmoid->Backprop(bottoms, tops);

  cout << "bottom size: " << bottom.data.shape_[0] << "x" << bottom.data.shape_[1] << "x" << bottom.data.shape_[2] << "x" << bottom.data.shape_[3] << endl;
  cout << "top size: " << top.data.shape_[0] << "x" << top.data.shape_[1] << "x" << top.data.shape_[2] << "x" << top.data.shape_[3] << endl;
  
  cout << "top data: " << top.data[0][0][0][0] << endl;
  cout << "bottom diff: " << bottom.diff[0][0][0][0] << endl;
}

int main(int argc, char *argv[]) {
  mshadow::Random<cpu> rnd(37);
  TestCrossLayer(&rnd);
  return 0;
}


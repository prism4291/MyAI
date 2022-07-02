# include <Siv3D.hpp> // OpenSiv3D v0.6.4

class NeuralNetwork {
public:
	size_t layerCount;
	Array<int32> nodeCount;
	Array<Array<double>> layer;
	Array<Array<Array<double>>> weightData;
	Array<Array<double>> biasData;
	NeuralNetwork(const Array<int32>& nodes) {
		nodeCount = nodes;
		layerCount = nodeCount.size();
		layer = {};
		for (size_t i = 0; i < layerCount; ++i) {
			layer << Array<double>{};
			for (auto j = 0; j < nodeCount[i]; ++j) {
				layer[i] << 0.0;
			}
		}
		weightData = {};
		biasData = {};
		for (size_t i = 0; i < layerCount - 1; ++i) {
			weightData << Array<Array<double>>{};
			for (auto j = 0; j < nodeCount[i]; ++j) {
				weightData[i] << Array<double>{};
				for (auto k = 0; k < nodeCount[i + 1]; ++k) {
					weightData[i][j] << 0.0;
				}
			}
			biasData << Array<double>{};
			for (auto k = 0; k < nodeCount[i + 1]; ++k) {
				biasData[i] << 0.0;
			}
		}
	}
	void setRandom() {
		for (size_t i = 0; i < layerCount - 1; ++i) {
			for (auto j = 0; j < nodeCount[i]; ++j) {
				for (auto k = 0; k < nodeCount[i + 1]; ++k) {
					weightData[i][j][k] = Random(-1.0, 1.0);
				}
			}
			for (auto k = 0; k < nodeCount[i + 1]; ++k) {
				biasData[i][k] = Random(-1.0, 1.0);
			}
		}
	}
	Array<double> forward(const Array<double>& in) {
		layer[0] = in;
		for (size_t i = 0; i < layerCount - 1; ++i) {
			for (int32 j = 0; j < nodeCount[i + 1]; ++j) {
				layer[i + 1][j] = 0.0;
				for (int32 k = 0; k < nodeCount[i]; ++k) {
					layer[i + 1][j] += layer[i][k] * weightData[i][k][j];
				}
				layer[i + 1][j] += biasData[i][j];
				if (i < 2) {
					layer[i + 1][j] = Max(layer[i + 1][j], 0.0);
				}
			}
		}
		return layer[layerCount - 1];
	}
	void back(const Array<double>& err) {
		Array<double> error = err;
		Array<Array<Array<double>>> weightDelta;
		Array<Array<double>> biasDelta;
		for (size_t i = 0; i < layerCount - 1; ++i) {
			weightDelta << Array<Array<double>>{};
			for (auto j = 0; j < nodeCount[i]; ++j) {
				weightDelta[i] << Array<double>{};
				for (auto k = 0; k < nodeCount[i + 1]; ++k) {
					weightDelta[i][j] << 0.0;
				}
			}
			biasDelta << Array<double>{};
			for (auto k = 0; k < nodeCount[i + 1]; ++k) {
				biasDelta[i] << 0.0;
			}
		}
		for (size_t ii = 0; ii < layerCount - 1;++ii) {
			size_t i = layerCount - ii - 2;
			for (int32 j = 0; j < nodeCount[i+1]; ++j) {
				double dfdw = 1;
				if (i < 2) {
					if (layer[i+1][j] < 0) {
						dfdw = 0;
					}
				}
				for (int32 k = 0; k < nodeCount[i]; ++k) {
					
					 weightDelta[i][k][j] += error[j] * dfdw * layer[i][k];
				}
				biasDelta[i][j]+= error[j] * dfdw;
			}
			Array<double> error2 = Array<double>{};
			for (int32 j = 0; j < nodeCount[i]; ++j) {
				error2 << 0.0;
					for (int32 k = 0; k < nodeCount[i+1]; ++k) {
						double dfdw = 1;
						if (i < 2) {
							if (layer[i + 1][k] < 0) {
								dfdw = 0;
							}
						}
						error2[j] += error[k] * dfdw * weightData[i][j][k];
					}
			}
			error = error2;
			
		}
		for (size_t i = 0; i < layerCount - 1; ++i) {
			for (auto j = 0; j < nodeCount[i]; ++j) {
				for (auto k = 0; k < nodeCount[i + 1]; ++k) {
					weightData[i][j][k] += weightDelta[i][j][k];
				}
			}
			for (auto k = 0; k < nodeCount[i + 1]; ++k) {
				biasData[i][k]+=biasDelta[i][k];
			}
		}
	}
};
class Player {
public:
	int32 power;
	NeuralNetwork nn = NeuralNetwork{ Array<int32>{1,3,3,2} };
	NeuralNetwork nn_t = NeuralNetwork{ Array<int32>{1,3,3,2} };
	Array<int32> action;
	Player() {
		power = 0;
	}
	Array<double> getData() {
		Array<double> data = {};
		data << power;
		return data;
	}
	double move(const double deltaTime, const Array<int32>& in) {
		double reward = 0;
		if (in[0] == 0) {
			if (power == 0) {
				power = 1;
			}
			else if (power == 1) {
				power = 0;
				//reward -= 1.0;
			}
		}
		if (in[0] == 1) {
			if (power == 1) {
				reward += 1.0;
			}
		}
		return reward;
	}

	void main(const double deltaTime) {
		Array<double> state1 = getData();
		Array<double> out1 = nn.forward(state1);
		action = {};
		action << Random(0, 1);
		if (Random(1.0) > 0.8) {
			if (out1[0] > out1[1]) {
				action[0] = 0;
			}
			else {
				action[0] = 1;
			}
		}
		double current_Q = out1[action[0]];
		double reward = move(deltaTime, action);
		Array<double> state2 = getData();
		Array<double> out2 = nn_t.forward(state2);
		double next_Q_max = out2.rsorted()[0];
		Array<double> out_error = {};
		for (int32 i = 0; i < 2; ++i) {
			out_error << 0.0;
			if (i == action[0]) {
				out_error[i] = 0.0001 * (reward + 0.9* next_Q_max - current_Q);
				/*
				if (out_error[i] > 1.0) {
					out_error[i] = 1.0;
				}
				if (out_error[i] < -1.0) {
					out_error[i] = -1.0;
				}
				*/
			}
		}
		nn.back(out_error);
		//nn.back(out_error);
		//Array<double> out3 = nn.forward(state1);
	}
	void syncTarget() {
		nn_t = nn;
	}
};

void Main()
{
	Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });
	const Font font{ 30 };
	Player player;
	player.nn.setRandom();
	player.syncTarget();
	//Print << player.nn.weightData;
	//player.main(Scene::DeltaTime());
	//player.power = 1;
	//player.main(Scene::DeltaTime());
	int32 loop = 1;
	while (System::Update())
	{
		if (MouseL.pressed()) {
			loop = 100;
		}
		else {
			loop = 1;
		}
		for (auto t = 0; t < loop; ++t) {
			for (auto i = 0; i < 100; ++i) {
				player.main(Scene::DeltaTime());

			}
			Print << U"action" << player.action << U"power off " << player.nn.forward(Array<double>{0.0}) << U"power on  " << player.nn.forward(Array<double>{1.0});
			player.syncTarget();
		}
	}
}

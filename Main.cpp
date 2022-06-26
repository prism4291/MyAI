# include <Siv3D.hpp> // OpenSiv3D v0.6.4

struct WeightValue {
	double Multi=0.0;
	double Add=0.0;
};
struct Weight {
	Array<Array<WeightValue>> value = {};
	int32 fromCount;
	int32 toCount;
	Weight(int32 from, int32 to) {
		fromCount = from;
		toCount = to;
		for (auto i = 0; i < fromCount; ++i) {
			value << Array<WeightValue>{};
			for (auto j = 0; j < toCount; ++j) {
				value[i] << WeightValue{};
			}
		}
	}
	void setRandom() {
		for (auto i = 0; i < fromCount; ++i) {
			for (auto j = 0; j < toCount; ++j) {
				value[i][j].Multi = Random(-1.0,1.0)/fromCount;
				value[i][j].Add = Random(-1.0, 1.0)/fromCount;
			}
		}
	}
};
struct NetWork {
	Array<Weight> weights;
	Array<int32> nodeCount;
	NetWork(Array<int32>& nodes) {
		nodeCount = nodes;
		for (int32 i = 0; i < nodeCount.size()-1; ++i) {
			weights << Weight{ nodeCount[i], nodeCount[i + 1] };
		}
	}
	void setRandom() {
		for (int32 i = 0; i < nodeCount.size() - 1; ++i) {
			weights[i].setRandom();
		}
	}
};
class Player {
public:
	Vec2 pos=Scene::Center();
	Vec2 vel ={ 0.0,0.0 };
	double ang = 0;
	const double rad = 10;
	const double accel_speed = 120;
	const double turn_speed = 2_pi;
	Array<int32> nodes={ 2,6,3 };
	NetWork netWork={ nodes};
	void reset() {
		pos = Scene::Center();
		vel = { 0.0,0.0 };
		ang = 0;
	}
	void randomWeight() {
		netWork.setRandom();
	};
	Array<double> getOut(Array<double>& in) {
		Array<double> layerIn;
		Array<double> layerOut;
		for (int32 i = 0; i < nodes.size() - 1; ++i) {
			if (i == 0) {
				layerIn = in;
			}
			else {
				layerIn = layerOut;
			}
			layerOut = {};
			for (int32 j = 0; j < nodes[i + 1]; ++j) {
				layerOut << 0.0;
				for (int32 k = 0; k < nodes[i]; ++k) {
					layerOut[j] += layerIn[k] * netWork.weights[i].value[k][j].Multi + netWork.weights[i].value[k][j].Add;
				}
				layerOut[j] = 1 / (1 + Exp(-layerOut[j]));
				/*if (layerOut[j] < 0) {
					layerOut[j] = 0;
				}*/
			}
		}
		/*for (int32 j = 0; j < nodes[nodes.size() - 1]; ++j) {
			layerOut[j] = 1 / (1+Exp(-layerOut[j]));
		}*/
		return layerOut;
	}
	void move(double deltaTime,HashTable<String, bool>& in) {
		if (in[U"right"]) {
			ang += turn_speed * deltaTime;
		}
		if (in[U"left"]) {
			ang -= turn_speed * deltaTime;
		}
		if (in[U"accel"]) {
			vel += Vec2{ Cos(ang),Sin(ang) }*accel_speed * deltaTime;
		}
		vel -= vel*0.1*deltaTime;
		if (vel.length() > 1000) {
			vel = vel * 1000 / vel.length();
		}
		pos += vel * deltaTime;
		if (pos.x < 0) {
			pos.x = 0;
			vel.x = 0;
		}
		if (pos.y < 0) {
			pos.y = 0;
			vel.y = 0;
		}
		if (pos.x > Scene::Width()) {
			pos.x = Scene::Width();
			vel.x = 0;
		}
		if (pos.y > Scene::Height()) {
			pos.y = Scene::Height();
			vel.y = 0;
		}
	}
	void draw() const{
		Circle{ pos,rad }.draw(Palette::Orange);
		Line{ pos,pos + Vec2{ Cos(ang),Sin(ang) } * rad * 2 }.draw(Palette::Orange);
	}
};
void Main()
{
	Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });
	Player player;
	Array<Player> players;
	HashTable<String, bool> inputs = { {U"accel",false},{U"right",false} ,{U"left",false} };
	Array<int32> layerCount = { 8,10,3 };
	for (auto i = 0; i < 100; ++i) {
		players << Player();
		players[i].randomWeight();
	}
	Circle circle = { 600,400,50 };
	//Array<double> input_layer = { 0.0,0.0 };
	//Print << players[0].getOut(input_layer);
	for (int32 steps = 0; steps < 300; ++steps) {
		for (int32 step = 0; step < 100; ++step) {
			for (auto p = 0; p < players.size(); ++p) {
				Array<double> input_layer = {};
				double in1 = players[p].pos.x / Scene::Width();
				input_layer << in1;
				double in2 = players[p].pos.y / Scene::Height();
				input_layer << in2;
				double in3 = players[p].vel.length() / 1000;
				input_layer << in3;
				for (int k = -2; k <= 2; ++k) {
					Line arrow = { players[p].pos.x,players[p].pos.y,players[p].pos.x + Cos(players[p].ang+k) * 300,players[p].pos.y + Sin(players[p].ang+k) * 300 };
					if (arrow.intersects(circle)) {
						auto arrow_distance = arrow.intersectsAt(circle).value()[0].distanceFrom(players[p].pos) / 300;
						input_layer << arrow_distance;
					}
					else {
						input_layer << 1;
					}
				}
				Array<double> output_layer = players[p].getOut(input_layer);
				inputs[U"accel"] = (output_layer[0] >= 0.5);
				inputs[U"right"] = (output_layer[1] >= 0.5);
				inputs[U"left"] = (output_layer[2] >= 0.5);
				//inputs[U"accel"] = RandomBool();
				//inputs[U"right"] = RandomBool();
				//inputs[U"left"] = RandomBool();
				players[p].move(0.1, inputs);
			}
		}
		
		Array<double> player_points;
		HashTable<int32, int32> tab;
		for (int32 p = 0; p < players.size(); ++p) {
			player_points<< players[p].vel.length() -players[p].pos.distanceFromSq(600, 400) ;
			tab[p] = p;
			for (int32 p2 = 0; p2 < p; ++p2) {
				if (player_points[p] > player_points[tab[p2]]) {
					for (int32 p3 = p; p3 >p2; --p3) {
						tab[p3] = tab[p3-1];
					}
					tab[p2] = p;
					break;
				}
			}
		}
		for (int32 p = 0; p < 30; ++p) {
			players[tab[p]].reset();
		}
		for (int32 p = 30; p < players.size(); ++p) {
			players[tab[p]].reset();
			players[tab[p]].randomWeight();
		}
	}
	while (System::Update())
	{

		
		inputs[U"accel"] = KeyUp.pressed();
		inputs[U"right"] = KeyRight.pressed();
		inputs[U"left"] = KeyLeft.pressed();
		player.move(Scene::DeltaTime(), inputs);
		player.draw();
		circle.draw(Palette::White);
		for (auto p = 0; p < players.size();++p) {
			Array<double> input_layer = {};
			double in1 = players[p].pos.x/Scene::Width();
			input_layer << in1;
			double in2 = players[p].pos.y / Scene::Height();
			input_layer << in2;
			double in3 = players[p].vel.length()/1000;
			input_layer << in3;
			for (int k = -2; k <= 2; ++k) {
				Line arrow = { players[p].pos.x,players[p].pos.y,players[p].pos.x + Cos(players[p].ang+k) * 300,players[p].pos.y + Sin(players[p].ang+k) * 300 };
				if (arrow.intersects(circle)) {
					auto arrow_distance = arrow.intersectsAt(circle).value()[0].distanceFrom(players[p].pos) / 300;
					input_layer << arrow_distance;
				}
				else {
					input_layer << 1;
				}
			}

			Array<double> output_layer = players[p].getOut(input_layer);
			inputs[U"accel"] = (output_layer[0]>=0.5);
			inputs[U"right"] = (output_layer[1]>=0.5);
			inputs[U"left"] = (output_layer[2]>=0.5);
			players[p].move(Scene::DeltaTime(), inputs);
			players[p].draw();
		}
		
	}
}

//
// - Debug ビルド: プログラムの最適化を減らす代わりに、エラーやクラッシュ時に詳細な情報を得られます。
//
// - Release ビルド: 最大限の最適化でビルドします。
//
// - [デバッグ] メニュー → [デバッグの開始] でプログラムを実行すると、[出力] ウィンドウに詳細なログが表示され、エラーの原因を探せます。
//
// - Visual Studio を更新した直後は、プログラムのリビルド（[ビルド]メニュー → [ソリューションのリビルド]）が必要です。
//
// Siv3D リファレンス
// https://zenn.dev/reputeless/books/siv3d-documentation
//
// Siv3D Reference
// https://zenn.dev/reputeless/books/siv3d-documentation-en
//
// Siv3D コミュニティへの参加（Slack や Twitter, BBS で気軽に質問や情報交換ができます）
// https://zenn.dev/reputeless/books/siv3d-documentation/viewer/community
//
// Siv3D User Community
// https://zenn.dev/reputeless/books/siv3d-documentation-en/viewer/community
//
// 新機能の提案やバグの報告 | Feedback
// https://github.com/Siv3D/OpenSiv3D/issues
//
// Sponsoring Siv3D
// https://github.com/sponsors/Reputeless
//

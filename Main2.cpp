# include <Siv3D.hpp> // OpenSiv3D v0.6.4
struct WeightValue {
	double w;
	double b;
};
struct Weight {
	int32 fromCount;
	int32 toCount;
	Array<Array<WeightValue>> value;
	Weight(const int32 from, const int32 to) {
		fromCount = from;
		toCount = to;
		value = {};
		for (int32 i = 0; i < fromCount; ++i) {
			value << Array<WeightValue>{};
			for (int32 j = 0; j < toCount; ++j) {
				value[i] << WeightValue{};
			}
		}
	}
	void setRandom() {
		for (int32 i = 0; i < fromCount; ++i) {
			for (int32 j = 0; j < toCount; ++j) {
				value[i][j].w = Random(-1.0, 1.0) / fromCount;
				value[i][j].b = Random(-1.0, 1.0);
			}
		}
	}
};
struct NeuralNetwork {
	int32 layerCount;
	Array<int32> nodeCount;
	Array<Weight> weights;
	NeuralNetwork(const Array<int32> &nodes) {
		layerCount = nodes.size();
		nodeCount = nodes;
		weights = {};
		for (int32 i = 0; i < layerCount - 1;++i) {
			weights << Weight{ nodeCount[i],nodeCount[i + 1] };
		}
	}
	void setRandom() {
		for (int32 i = 0; i < layerCount - 1; ++i) {
			weights[i].setRandom();
		}
	}
	Array<double> getOutput(const Array<double>& input) {
		Array<double> fromLayer = {};
		Array<double> toLayer = {};
		for (int32 i = 0; i < layerCount - 1; ++i) {
			if (i == 0) {
				fromLayer = input;
			}
			else {
				fromLayer = toLayer;
			}
			toLayer = {};
			for (int32 j = 0; j < nodeCount[i + 1]; ++j) {
				double val=0;
				for (int32 k = 0; k < nodeCount[i ]; ++k) {
					val += fromLayer[k] * weights[i].value[k][j].w + weights[i].value[k][j].b;
				}
				toLayer << 1/(1+Exp(-val));
			}
		}
		return toLayer;
	}

};
struct PlayerInput {
	bool right;
	bool left;
	bool up;
	bool down;
	PlayerInput(const bool r, const bool l, const bool u, const bool d) {
		right = r;
		left = l;
		up = u;
		down = d;
	}
	PlayerInput(const Array<bool> &input) {
		right = input[0];
		left = input[1];
		up = input[2];
		down = input[3];
	}
};
class Player {
public:
	Vec2 pos;
	Vec2 vel;
	Vec2 acc;
	const double maxSpeed = 800;
	const double rad=16;
	const Color circleColor ={ Palette::Orange.r,Palette::Orange.g,Palette::Orange.b,128U };
	double reward;
	NeuralNetwork neuralNetwork={Array<int32>{6,20,4}};
	Vec2 target = { 200.0,200.0 };
	Player() {
		pos = Scene::Center();
		vel = Vec2();
		acc = Vec2();
		reward = 0;
		neuralNetwork.setRandom();
	}
	void move(const double deltaTime, const PlayerInput &input) {
		acc.x = 0;
		acc.y = 400;
		if (input.right&&not input.left) {
			acc.x += 500;
		}
		if (input.left&&not input.right) {
			acc.x -= 500;
		}
		if (input.up && not input.down) {
			acc.y -= 100;
		}
		if (input.down && not input.up) {
			acc.y += 100;
		}
		vel += acc*deltaTime;
		vel -= vel * 0.1 * deltaTime;
		vel.x = Clamp(vel.x, -maxSpeed, maxSpeed);
		vel.y = Clamp(vel.y, -maxSpeed, maxSpeed);
		pos += vel*deltaTime;
		if (pos.y + rad > Scene::Height()) {
			pos.y = Scene::Height() * 2 - (pos.y + rad)-rad;
			vel.y = -vel.y;
		}
		reward -= pos.distanceFrom(target)*deltaTime;
	}
	Array<double> getData() {
		Array<double> data = {};
		data << pos.x / Scene::Width();
		data << pos.y / Scene::Height();
		data << vel.x / maxSpeed;
		data << vel.y / maxSpeed;
		data << target.x / Scene::Width();
		data << target.y / Scene::Height();
		return data;
	}
	void moveWithNetwork(const double deltaTime) {
		Array<double> networkOutput = neuralNetwork.getOutput(getData());
		Array<bool> playerInput = {};
		for (double d : networkOutput) {
			if (d >= 0.5) {
				playerInput << true;
			}
			else {
				playerInput << false;
			}
		}
		move(deltaTime, PlayerInput{ playerInput });

	}
	void draw() {
		Circle{ pos,rad }.draw(circleColor);
	}
};



void Main()
{
	Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });

	Array<Player> players;
	for (int32 i = 0; i < 100; ++i) {
		players << Player{};
	}
	int32 fps = 10;
	for (int32 step = 0; step < 100; ++step) {
		for (int32 t = 0; t < 10*fps; ++t) {
			for (int32 i = 0; i < players.size(); ++i) {
				players[i].moveWithNetwork(Scene::DeltaTime());
			}
		}
	}
	while (System::Update())
	{
		//Print << players[0].getData();
		//player.move(Scene::DeltaTime(), PlayerInput{ KeyRight.pressed(),KeyLeft.pressed(),KeyUp.pressed(),KeyDown.pressed() });
		for (int32 i = 0; i < players.size(); ++i) {
			players[i].moveWithNetwork(Scene::DeltaTime());
			players[i].draw();
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

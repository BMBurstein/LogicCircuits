#include <memory>
#include <vector>

class Gate {
public:
	typedef /*std::shared_ptr<Gate>*/ Gate* ptr;

private:
	class Conn {
		bool _state;
		Gate::ptr _p;
	public:
		typedef std::shared_ptr<Conn> ptr;

		Conn() : _p(nullptr) { };
		Conn(Gate::ptr p) : _p(p) { };
		void connect_to(Gate::ptr p) { _p = p; }
		//void set(bool val);
		//bool state() const { return _state; }
		Conn& operator=(bool val) {
			_state = val;
			if (_p) {
				_p->eval();
			}
			return *this;
		}
		explicit operator bool() const { return _state; }
	};

	class Connectors {
		std::vector<Conn::ptr> _conns;
	public:
		Connectors(std::size_t num) : _conns(num) {
			for (auto& conn : _conns) {
				conn = std::make_shared<Conn>();
			}
		}
		Conn::ptr& operator[](std::size_t i) {
			return _conns[i];
		}
		Conn::ptr const& operator[](std::size_t i) const {
			return _conns[i];
		}
		std::size_t size() const { return _conns.size(); }
	};

protected:
	Connectors _inputs;

public:
	Connectors const& inputs;
	Connectors outputs;

	Gate(std::size_t num_inputs, std::size_t num_outputs) : _inputs(num_inputs), inputs(_inputs), outputs(num_outputs) {
		for (std::size_t i = 0; i < num_inputs; ++i) {
			inputs[i]->connect_to(this);
		}
	}
	virtual ~Gate() = 0 { };

	virtual void eval() { };
};

class NotGate : public Gate {
public:
	NotGate() : Gate(1, 1) { }
	void eval() {
		*outputs[0] = !*inputs[0];
	}
};

class AndGate : public Gate {
public:
	AndGate() : Gate(2, 1) { }
	void eval() {
		*outputs[0] = *inputs[0] && *inputs[1];
	}
};

class NandGate : public Gate {
	AndGate and1;
	NotGate not1;
public:
	NandGate() : Gate(2, 1) {
		_inputs[0] = and1.inputs[0];
		_inputs[1] = and1.inputs[1];
		and1.outputs[0] = not1.inputs[0];
		outputs[0] = not1.outputs[0];
	}
};

class NotGate2 : public Gate {
	NandGate nand1;
	NandGate nand2;
	NandGate nand3;
public:
	NotGate2() : Gate(1, 1) {
		_inputs[0] = nand1.inputs[0];
		*nand1.inputs[1] = true;
		nand1.outputs[0] = nand2.inputs[0];
		*nand2.inputs[1] = true;
		nand2.outputs[0] = nand3.inputs[0];
		*nand3.inputs[1] = true;
		outputs[0] = nand3.outputs[0];
	}
};

#include <iostream>
using namespace std;

int main() {
	//cout << std::boolalpha;

	NotGate2 not1;
	*not1.inputs[0] = false;
	cout << "not(false) = " << bool(*not1.outputs[0]) << '\n';
	*not1.inputs[0] = true;
	cout << "not(true) = " << bool(*not1.outputs[0]) << '\n';
	*not1.inputs[0] = false;
	cout << "not(false) = " << bool(*not1.outputs[0]) << '\n';


	AndGate and1;
	*and1.inputs[0] = false;
	*and1.inputs[1] = false;
	cout << "and(false, false) = " << bool(*and1.outputs[0]) << '\n';
	*and1.inputs[0] = true;
	*and1.inputs[1] = false;
	cout << "and(true, false) = " << bool(*and1.outputs[0]) << '\n';
	*and1.inputs[0] = true;
	*and1.inputs[1] = true;
	cout << "and(true, true) = " << bool(*and1.outputs[0]) << '\n';


	NandGate nand1;
	*nand1.inputs[0] = false;
	*nand1.inputs[1] = false;
	cout << "nand(false, false) = " << bool(*nand1.outputs[0]) << '\n';
	*nand1.inputs[0] = true;
	*nand1.inputs[1] = false;
	cout << "nand(true, false) = " << bool(*nand1.outputs[0]) << '\n';
	*nand1.inputs[0] = true;
	*nand1.inputs[1] = true;
	cout << "nand(true, true) = " << bool(*nand1.outputs[0]) << '\n';
	return 0;
}
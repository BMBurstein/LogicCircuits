#include <memory>
#include <vector>

class Gate {
public:
	typedef /*std::shared_ptr<Gate>*/ Gate* ptr;

protected:
	class Connector {
		bool _state;
		Gate::ptr _p;
		std::size_t _input_num;
	public:
		typedef std::shared_ptr<Connector> ptr;

		Connector() : _p(nullptr) { };
		Connector(Gate::ptr p, std::size_t num) : _p(p), _input_num(num) { };
		void connect_to(Gate::ptr p, std::size_t num) { _p = p; _input_num = num; }
		Connector& operator=(Connector const& other) { return operator=(other._state); }
		Connector& operator=(bool val) {
			_state = val;
			if (_p) {
				if (&_p->inputs[_input_num] != this) {
					_p->inputs[_input_num] = val;
				}
				_p->eval();
			}
			return *this;
		}
		operator bool() const { return _state; }
	};

public:
	std::unique_ptr<Connector[]> const inputs;
	std::unique_ptr<Connector[]> const outputs;

	Gate(std::size_t num_inputs, std::size_t num_outputs) : inputs(std::make_unique<Connector[]>(num_inputs)), outputs(std::make_unique<Connector[]>(num_outputs)) {
		for (std::size_t i = 0; i < num_inputs; ++i) {
			inputs[i].connect_to(this, i);
		}
	}
	virtual ~Gate() = 0 { };

	virtual void eval() { }
};

class GateTerminator : public Gate {
public:
	Connector& p_output;
	GateTerminator(Connector& conn) : Gate(1, 0), p_output(conn) { }
	void eval() {
		p_output = inputs[0];
	}
};

class NotGate : public Gate {
public:
	NotGate() : Gate(1, 1) { }
	void eval() {
		outputs[0] = !inputs[0];
	}
};

class AndGate : public Gate {
public:
	AndGate() : Gate(2, 1) { }
	void eval() {
		outputs[0] = inputs[0] && inputs[1];
	}
};

class NandGate : public Gate {
	AndGate and1;
	NotGate not1;
	GateTerminator term;
public:
	NandGate() : Gate(2, 1), term(outputs[0]) {
		and1.outputs[0].connect_to(&not1, 0);
		inputs[0].connect_to(&and1, 0);
		inputs[1].connect_to(&and1, 1);
		not1.outputs[0].connect_to(&term, 0);
	}
};

class NotGate2 : public Gate {
	NandGate nand1;
	NandGate nand2;
	NandGate nand3;
	GateTerminator term;
public:
	NotGate2() : Gate(1, 1), term(outputs[0]) {
		inputs[0].connect_to(&nand1, 0);
		nand1.inputs[1] = true;
		nand1.outputs[0].connect_to(&nand2, 0);
		nand2.inputs[1] = true;
		nand2.outputs[0].connect_to(&nand3, 0);
		nand3.inputs[1] = true;
		nand3.outputs[0].connect_to(&term, 0);
	}
};

#include <iostream>
using namespace std;

int main() {
	//cout << std::boolalpha;

	NotGate2 not1;
	not1.inputs[0] = false;
	cout << "not(false) = " << bool(not1.outputs[0]) << '\n';
	not1.inputs[0] = true;
	cout << "not(true) = " << bool(not1.outputs[0]) << '\n';
	not1.inputs[0] = false;
	cout << "not(false) = " << bool(not1.outputs[0]) << '\n';


	AndGate and1;
	and1.inputs[0] = false;
	and1.inputs[1] = false;
	cout << "and(false, false) = " << bool(and1.outputs[0]) << '\n';
	and1.inputs[0] = true;
	and1.inputs[1] = false;
	cout << "and(true, false) = " << bool(and1.outputs[0]) << '\n';
	and1.inputs[0] = true;
	and1.inputs[1] = true;
	cout << "and(true, true) = " << bool(and1.outputs[0]) << '\n';


	NandGate nand1;
	nand1.inputs[0] = false;
	nand1.inputs[1] = false;
	cout << "nand(false, false) = " << bool(nand1.outputs[0]) << '\n';
	nand1.inputs[0] = true;
	nand1.inputs[1] = false;
	cout << "nand(true, false) = " << bool(nand1.outputs[0]) << '\n';
	nand1.inputs[0] = true;
	nand1.inputs[1] = true;
	cout << "nand(true, true) = " << bool(nand1.outputs[0]) << '\n';
	return 0;
}
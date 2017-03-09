#include <memory>
#include <vector>
#include <sstream>
#include <string>

class Gate {
public:
	typedef /*std::shared_ptr<Gate>*/ Gate* ptr;

protected:
	class Connector {
		bool _state;
		Gate::ptr _p;
		std::size_t _input_num;
	public:
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

	template <typename T>
	class FixedArray {
		std::unique_ptr<T[]> _p;
		std::size_t _size;
	public:
		FixedArray(std::size_t size) : _p(std::make_unique<T[]>(size)), _size(size) { }
		T& operator[](std::size_t index) { return _p[index]; }
		const T& operator[](std::size_t index) const { return _p[index]; }
		constexpr std::size_t size() const { return _size; }
		T* begin() { return _p.get(); }
		T* end() { return _p.get() + _size; }
	};

public:
	typedef FixedArray<Connector> ConnArray;
	ConnArray inputs;
	ConnArray outputs;

	Gate(std::size_t num_inputs, std::size_t num_outputs) : inputs(num_inputs), outputs(num_outputs) {
		for (std::size_t i = 0; i < num_inputs; ++i) {
			inputs[i].connect_to(this, i);
		}
	}
	Gate(ConnArray in, ConnArray out) : inputs(std::move(in)), outputs(std::move(out)) { }
	virtual ~Gate() = 0;

	virtual void eval() { }

	std::string truth_table() {
		for (auto& it : inputs) {
			it = false;
		}
		std::ostringstream os;
		for (std::size_t i = 0; i < (1 << inputs.size()); ++i) {
			for (auto& it : inputs) {
				os << it << '|';
			}for (auto& it : outputs) {
				os << '|' << it;
			}
			os << '\n';
			for (auto& it : inputs) {
				if (it) {
					it = false;
				}
				else {
					it = true;
					break;
				}
			}
		}
		return os.str();
	}
};

Gate::~Gate() { }

class GateTerminator : public Gate {
	Connector& p_output;
public:
	GateTerminator(Connector& conn) : Gate(1, 0), p_output(conn) { }
	~GateTerminator() { }
	void eval() {
		p_output = inputs[0];
	}
};

class GateCompose : public Gate {
	std::vector<std::unique_ptr<Gate>> _gates;
public:
	GateCompose(ConnArray in, ConnArray out, std::vector<std::unique_ptr<Gate>> gates) : Gate(std::move(in), std::move(out)), _gates(std::move(gates)) { }
	~GateCompose() { };
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
	cout << not1.truth_table();
	not1.inputs[0] = false;
	cout << "not(false) = " << bool(not1.outputs[0]) << '\n';
	not1.inputs[0] = true;
	cout << "not(true) = " << bool(not1.outputs[0]) << '\n';
	not1.inputs[0] = false;
	cout << "not(false) = " << bool(not1.outputs[0]) << '\n';

	AndGate and1;
	cout << and1.truth_table();
	and1.inputs[0] = false;
	and1.inputs[1] = false;
	cout << "and(false, false) = " << bool(and1.outputs[0]) << '\n';
	and1.inputs[0] = true;
	and1.inputs[1] = false;
	cout << "and(true, false) = " << bool(and1.outputs[0]) << '\n';
	and1.inputs[0] = true;
	and1.inputs[1] = true;
	cout << "and(true, true) = " << bool(and1.outputs[0]) << '\n';

	Gate::ConnArray ins(2);
	Gate::ConnArray outs(1);
	std::vector<std::unique_ptr<Gate>> gates;
	gates.emplace_back(std::make_unique<AndGate>());
	gates.emplace_back(std::make_unique<NotGate>());
	gates.emplace_back(std::make_unique<GateTerminator>(outs[0]));
	ins[0].connect_to(gates[0].get(), 0);
	ins[1].connect_to(gates[0].get(), 1);
	gates[0]->outputs[0].connect_to(gates[1].get(), 0);
	gates[1]->outputs[0].connect_to(gates[2].get(), 0);
	GateCompose nand1{ std::move(ins), std::move(outs), std::move(gates) };
	cout << nand1.truth_table();
	//NandGate nand1;
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

#include <memory>
#include <vector>

class Gate {
public:
	typedef /*std::shared_ptr<Gate>*/ Gate* ptr;

private:
	class ConnIn final {
		bool _state;
		Gate::ptr _p;
	public:
		typedef std::shared_ptr<ConnIn> ptr;

		ConnIn() { }
		ConnIn(Gate::ptr p) : _p(p) { }
		void connect_to(Gate::ptr p) { _p = p; }
		ConnIn& operator=(bool val) {
			_state = val;
			if (_p) {
				_p->eval();
			}
			return *this;
		}
		explicit operator bool() const { return _state; }
	};

	class ConnOut final {
		bool _state;
		ConnIn::ptr _p;
	public:
		ConnOut() { }
		ConnOut(ConnIn& p) : _p(&p) { }
		void connect_to(ConnIn::ptr p) { _p = p; }
		ConnOut& operator=(bool val) {
			_state = val;
			if (_p) {
				*_p = val;
			}
			return *this;
		}
		explicit operator bool() const { return _state; }
	};



	template <typename T>
	class Connectors {
		std::vector<T> _conns;
	public:
		Connectors(std::size_t num) : _conns(num) {	}
		T& operator[](std::size_t i) {
			return _conns[i];
		}
		T const& operator[](std::size_t i) const {
			return _conns[i];
		}
		std::size_t size() const { return _conns.size(); }
	};

protected:
	Connectors<ConnIn> _inputs;
	Connectors<ConnOut> _outputs;

public:
	Connectors<ConnIn> & inputs;
	Connectors<ConnOut> & outputs;

	Gate(std::size_t num_inputs, std::size_t num_outputs) : _inputs(num_inputs), _outputs(num_outputs), inputs(_inputs), outputs(_outputs) {
		for (std::size_t i = 0; i < num_inputs; ++i) {
			_inputs[i].connect_to(this);
		}
	}
	virtual ~Gate() = 0 { };

	virtual void eval() { };
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
		nand1.inputs[1] = true;
		nand1.outputs[0] = nand2.inputs[0];
		nand2.inputs[1] = true;
		nand2.outputs[0] = nand3.inputs[0];
		nand3.inputs[1] = true;
		outputs[0] = nand3.outputs[0];
	}
};

#include <iostream>
using namespace std;

int main() {
	//cout << std::boolalpha;

	NotGate not1;
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
//
// Created by Kamyar on 9/5/15.
//

#ifndef CHISEL_VERILATOR_VL_H
#define CHISEL_VERILATOR_VL_H

#include "sim_api.h"
#include "verilated.h"
#include <queue>


template<size_t pow2Bits>
struct vl_base_type{
};
template<>
struct vl_base_type<8>{
  typedef vluint8_t t;
};
template<>
struct vl_base_type<16>{
  typedef vluint16_t t;
};
template<>
struct vl_base_type<32>{
  typedef vluint32_t t;
};
template<>
struct vl_base_type<64>{
  typedef vluint64_t t;
};
template<>
struct vl_base_type<0>{
  typedef vluint32_t* t;
};

class vl_type_ {
protected:
public: // TODO for now
  size_t bits;
  std::string name;
public:
  vl_type_ (size_t bits, const std::string& name) : bits(bits), name(name) {}
  virtual void operator=(const std::string &hexString) = 0;
  virtual std::string toHex() = 0;
};

template<size_t N>
class vl_type : public vl_type_ {
  typename vl_base_type<N>::t& v;
public:
  vl_type(typename vl_base_type<N>::t& variable, size_t bits, const std::string& name): v(variable), vl_type_(bits, name) {}
  virtual void operator=(const std::string &hexString) {
    v = ((size_t(1) << N) - 1) & std::stoull(hexString, nullptr, 16);
  }

  virtual std::string toHex() {
    std::stringstream stream;
    stream << std::hex << static_cast<int64_t>(v);
    return stream.str();
  }
};


template<>
class vl_type<0> : public vl_type_ {
  vl_base_type<0>::t& v;
public:
  vl_type(typename vl_base_type<0>::t& variable, size_t bits, const std::string& name): v(variable), vl_type_(bits, name){}
  virtual void operator=(const std::string &hexString) {
    size_t words = hexString.length() / 8;
    size_t remain = hexString.length() % 8;
    for (size_t w = 0; w < words; ++w) {
      v[w] = std::stoull(hexString.substr(w * 8, 8), nullptr, 16);
    }
    if (remain != 0)
      v[words] = ((size_t(1) << remain) - 1) & std::stoull(hexString.substr(words * 8, remain), nullptr, 16);
  }

  virtual std::string toHex() {
    int words = bits / 32;
    std::stringstream stream;
    for (int i = 0; i < words; ++i) {
      stream << std::hex << v[i];
    }
    return stream.str();
  }
};

class vl_sig : public sim_signal {
protected:
  vl_type_ *sig_ptr;
public:

  vl_sig(typename vl_base_type<8>::t &variable, size_t bits, const std::string& name = "") : sig_ptr(new vl_type<8>(variable, bits, name)){}
  vl_sig(typename vl_base_type<16>::t &variable, size_t bits, const std::string& name = "") : sig_ptr(new vl_type<16>(variable, bits, name)){}
  vl_sig(typename vl_base_type<32>::t &variable, size_t bits, const std::string& name = "") : sig_ptr(new vl_type<32>(variable, bits, name)){}
  vl_sig(typename vl_base_type<64>::t &variable, size_t bits, const std::string& name = "") : sig_ptr(new vl_type<64>(variable, bits, name)){}
  vl_sig(typename vl_base_type<0>::t &variable, size_t bits, const std::string& name = "") : sig_ptr(new vl_type<0>(variable, bits, name)){}


  virtual bool put_value(const std::string &value) {
    *sig_ptr = value;
    return true;
  }

  virtual std::string get_value() {
    return sig_ptr->toHex();
  }

  virtual size_t get_width() {
    return sig_ptr->bits;
  }
};

template <typename T>
class vl_api_t : public sim_api_t<vl_sig*> {
protected:
    T* top;
public:
    virtual void start() {
      sim_api_t<vl_sig*>::start();
      top->eval();
    }
    vl_api_t(T *top): top(top){}

  void add_input(typename vl_base_type<0>::t &variable, size_t bits, const std::string& name = "") {
    sim_data.inputs.push_back(new vl_sig(variable, bits, name));
  }
  void add_input(typename vl_base_type<8>::t &variable, size_t bits, const std::string& name = "") {
    sim_data.inputs.push_back(new vl_sig(variable, bits, name));
  }
  void add_input(typename vl_base_type<16>::t &variable, size_t bits, const std::string& name = "") {
    sim_data.inputs.push_back(new vl_sig(variable, bits, name));
  }
  void add_input(typename vl_base_type<32>::t &variable, size_t bits, const std::string& name = "") {
    sim_data.inputs.push_back(new vl_sig(variable, bits, name));
  }
  void add_input(typename vl_base_type<64>::t &variable, size_t bits, const std::string& name = "") {
    sim_data.inputs.push_back(new vl_sig(variable, bits, name));
  }

  void add_output(typename vl_base_type<0>::t &variable, size_t bits, const std::string& name = "") {
      sim_data.outputs.push_back(new vl_sig(variable, bits, name));
  }
  void add_output(typename vl_base_type<8>::t &variable, size_t bits, const std::string& name = "") {
      sim_data.outputs.push_back(new vl_sig(variable, bits, name));
  }
  void add_output(typename vl_base_type<16>::t &variable, size_t bits, const std::string& name = "") {
      sim_data.outputs.push_back(new vl_sig(variable, bits, name));
  }
  void add_output(typename vl_base_type<32>::t &variable, size_t bits, const std::string& name = "") {
      sim_data.outputs.push_back(new vl_sig(variable, bits, name));
  }
  void add_output(typename vl_base_type<64>::t &variable, size_t bits, const std::string& name = "") {
      sim_data.outputs.push_back(new vl_sig(variable, bits, name));
  }

  void add_reset(typename vl_base_type<8>::t &variable, const std::string& name = "") {
    sim_data.resets.push_back(new vl_sig(variable, 1, name));
  }

  void add_clock(typename vl_base_type<8>::t &variable, const std::string name) {
    sim_data.clk_map.insert(std::make_pair(name, new vl_sig(variable, 1)));
  }

  void init_sigs() {
//    vpiHandle syscall_handle = vpi_handle(vpiSysTfCall, NULL);
//    top_handle = vpi_scan(vpi_iterate(vpiArgument, syscall_handle));
//    search_signals();
  }

protected:

  virtual void finish() { }

  virtual void step() {
    top->clk = 0;
    top->eval();
    top->clk = 1;
    top->eval();
  }

  virtual void update() {
  }


  virtual int search(std::string &wire) {
    std::cout << "in search(" + wire + ")" << std::endl;
    return -1;
  }
};


#endif //CHISEL_VERILATOR_VL_H

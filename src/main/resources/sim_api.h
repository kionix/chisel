#ifndef __SIM_API_H
#define __SIM_API_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

enum SIM_CMD { RESET, STEP, UPDATE, POKE, PEEK, GETID, SETCLK, FIN };

template<class T> struct sim_data_t {
  std::vector<T> resets;
  std::vector<T> inputs;
  std::vector<T> outputs;
  std::vector<T> signals;
  std::map<const std::string, size_t> signal_map;
  std::map<const std::string, T> clk_map;
};

class sim_signal {
public:
    virtual std::string get_value() = 0;
    virtual bool put_value(const std::string& value) = 0;
//    virtual void put_value(val_t value, size_t idx) = 0;
    virtual size_t get_width() = 0;
    virtual size_t get_num_words() {
      return ((get_width() - 1) >> 6) + 1;
    };
};

template <class T> class sim_api_t {
public:
  void tick() {

    static bool is_reset = false;
    // First, Generates output tokens  (in hex)
    generate_tokens();
    if (is_reset) {
      start();
      is_reset = false;
    }


    
    // Next, handle commands from the testers
    bool exit = false;
    do {
      size_t cmd;
      std::cin >> std::dec >> cmd;
      switch ((SIM_CMD) cmd) {
        case RESET:
          reset(); is_reset = true; exit = true; break;
        case STEP: 
          consume_tokens();
          step(); exit = true; break;
        case UPDATE: 
          consume_tokens();
          update(); exit = true; break;
        case POKE: poke(); break; 
        case PEEK: peek(); break;
        case GETID: getid(); break;
        case SETCLK: setclk(); break;
        case FIN:  finish(); exit = true; break;
        default: break;
      }
    } while (!exit);
  }
protected:
  virtual void reset() {
    for (size_t i = 0 ; i < sim_data.resets.size() ; i++) {
      sim_data.resets[i]->put_value("1");
    }
  }

  virtual void start() {
    for (size_t i = 0 ; i < sim_data.resets.size() ; i++) {
      sim_data.resets[i]->put_value("0");
    }
  }
  virtual void finish() = 0;
  virtual void update() = 0; 
  virtual void step() = 0;
  // Consumes input tokens (in hex)
  virtual void put_value(T sig) {
    std::string value;
    size_t words = sig->get_num_words();
    for (size_t k = 0 ; k < words  ; k++) {
      // 64 bit chunks are given
      std::string v;
      std::cin >> v;
      value += v;
    }
    sig->put_value(value);
  }

  // Generate output tokens (in hex)
  virtual void get_value(T sig) {
    std::cerr << sig->get_value() << std::endl;
  }


  // Find a signal of path 
  virtual int search(const std::string& /*path*/) { return -1; }

  void poke() {
    size_t id;
    std::cin >> std::dec >> id;
    try{
      put_value(sim_data.signals.at(id));
    } catch(std::out_of_range&) {
      std::cout << "Cannot find the object of id = " << id << std::endl;
      finish();
      exit(0);
    }
  }

  void peek() {
    size_t id;
    std::cin >> std::dec >> id;
    try{
      get_value(sim_data.signals.at(id));
    } catch(std::out_of_range&) {
      std::cout << "Cannot find the object of id = " << id << std::endl;
      finish();
      exit(0);
    }
  }

  void getid() {
    std::string wire;
    std::cin >> wire;
    auto it = sim_data.signal_map.find(wire);
    if (it != sim_data.signal_map.end()) {
      std::cerr << it->second << std::endl;
    } else {
      int id = search(wire);
      if (id < 0) {
        std::cout << "Cannot find the object, " << wire<< std::endl;
        finish();
        exit(0);
      }
      std::cerr << id << std::endl;
    }
  }

  void setclk() {
    std::string clkname;
    std::cin >> clkname;
    auto it = sim_data.clk_map.find(clkname);
    if (it != sim_data.clk_map.end()) {
      put_value(it->second);
    } else {
      std::cout << "Cannot find " << clkname << std::endl;
    }
  }

  void consume_tokens() {
    for (size_t i = 0 ; i < sim_data.inputs.size() ; i++) {
      put_value(sim_data.inputs[i]);
    }
  }

  void generate_tokens() {
    for (size_t i = 0 ; i < sim_data.outputs.size() ; i++) {
      get_value(sim_data.outputs[i]);
    }
  }
protected:
  sim_data_t<T> sim_data;

  virtual size_t add_signal(T sig, const std::string& wire) {
    size_t id = sim_data.signals.size();
    sim_data.signals.push_back(sig);
    sim_data.signal_map[wire] = id;
    return id;
  }

  void read_signal_map(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file) {
      std::cout << "Cannot open " << filename << std::endl;
      finish();
      exit(0);
    } 
    std::string line;
    size_t id = 0;
    while (std::getline(file, line)) {
      std::istringstream iss(line);
      std::string path;
      size_t width, n;
      iss >> path >> width >> n;
      sim_data.signal_map[path] = id;
      id += n;
    }
  }
};

#endif //__SIM_API_H

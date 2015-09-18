#ifndef __VPI_H
#define __VPI_H

#include "vpi_user.h"
#include "sim_api.h"
#include <queue>

PLI_INT32 tick_cb(p_cb_data cb_data);

class vpi_sig : public sim_signal {
  vpiHandle sig;
public:
  vpi_sig(vpiHandle handle) : sig(handle) { }

  virtual bool put_value(const std::string& value) {
    s_vpi_value value_s;
    value_s.format = vpiHexStrVal;
    value_s.value.str = (PLI_BYTE8 *) value.c_str();
    vpi_put_value(sig, &value_s, NULL, vpiNoDelay);
    return true;
  }

  virtual std::string get_value() {
    s_vpi_value value_s;
    value_s.format = vpiHexStrVal;
    vpi_get_value(sig, &value_s);
    return std::string(value_s.value.str);
  }

  virtual size_t get_width() {
    return (size_t) vpi_get(vpiSize, sig);
  }
};


class vpi_api_t : public sim_api_t<vpi_sig*> {
public:
    virtual void init_clks() {
    vpiHandle syscall_handle = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle arg_iter = vpi_iterate(vpiArgument, syscall_handle);
    // Cache clocks
    vpiHandle arg_handle;
    while (arg_iter && (arg_handle = vpi_scan(arg_iter))) {
      std::string name = vpi_get_str(vpiName, arg_handle);
      sim_data.clk_map.insert(std::make_pair(name.substr(0, name.rfind("_len")), new vpi_sig(arg_handle) ));
    }
  }

  virtual void init_rsts() {
    vpiHandle syscall_handle = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle arg_iter = vpi_iterate(vpiArgument, syscall_handle);
    // Cache Resets
    vpiHandle arg_handle;
    while (arg_iter && (arg_handle = vpi_scan(arg_iter))) {
      sim_data.resets.push_back(new vpi_sig(arg_handle));
    }
  }

    virtual void init_ins() {
    vpiHandle syscall_handle = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle arg_iter = vpi_iterate(vpiArgument, syscall_handle);
    // Cache Inputs
    vpiHandle arg_handle;
    while (arg_iter && (arg_handle = vpi_scan(arg_iter))) {
      sim_data.inputs.push_back(new vpi_sig(arg_handle));
    }
  }

  virtual void init_outs() {
    vpiHandle syscall_handle = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle arg_iter = vpi_iterate(vpiArgument, syscall_handle);
    // Cache Outputs
    vpiHandle arg_handle;
    while (arg_iter && (arg_handle = vpi_scan(arg_iter))) {
      sim_data.outputs.push_back(new vpi_sig(arg_handle));
    }
  }

  virtual void init_sigs() {
    vpiHandle syscall_handle = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle arg_iter = vpi_iterate(vpiArgument, syscall_handle);
    top_handle = vpi_scan(arg_iter);
    search_signals();
  }

private:
  vpiHandle top_handle;

  virtual void finish() { vpi_control(vpiFinish, 0); }

  virtual void step() { }

  virtual void update() {
    s_cb_data data_s;
    s_vpi_time time_s;
    time_s.type      = vpiSimTime;
    time_s.low       = 0;
    time_s.high      = 0;
    data_s.reason    = cbReadWriteSynch;
    data_s.cb_rtn    = tick_cb;
    data_s.obj       = NULL;
    data_s.time      = &time_s;
    data_s.value     = NULL;
    data_s.user_data = NULL;
    vpi_free_object(vpi_register_cb(&data_s));
  }

  int search_signals(const char *wire = NULL) {
    int id = -1;
    std::string wirepath = wire ? wire : "";
    int dotpos = wirepath.rfind(".");
    std::string modpath = dotpos > 0 ? wirepath.substr(0, dotpos) : "";
    std::string wirename = dotpos > 0 ? wirepath.substr(dotpos+1) : "";
    int sbrpos = wirename.rfind("[");
    std::string arrname = sbrpos > 0 ? wirename.substr(0, sbrpos) : "";
    std::queue<vpiHandle> modules;
    size_t offset = std::string(vpi_get_str(vpiFullName, top_handle)).find(".") + 1;

    // Start from the top module
    modules.push(top_handle);

    while (!modules.empty()) {
      vpiHandle mod_handle = modules.front();
      modules.pop();

      std::string modname = std::string(vpi_get_str(vpiFullName, mod_handle)).substr(offset);
      // If the module is found
      if (!wire || modpath == modname) {
        // Iterate its nets
        vpiHandle net_iter = vpi_iterate(vpiNet, mod_handle);
        vpiHandle net_handle;
        while (net_iter &&  (net_handle = vpi_scan(net_iter))) {
          std::string netname = vpi_get_str(vpiName, net_handle);
          std::string netpath = modname + "." + netname;
          size_t netid = (!wire && netname[0] != 'T') || wirename == netname ? 
            add_signal(new vpi_sig(net_handle), netpath) : 0;
          id = netid ? netid : id;
          if (id > 0) break;
        }
        if (id > 0) break;

        // Iterate its regs
        vpiHandle reg_iter = vpi_iterate(vpiReg, mod_handle);
        vpiHandle reg_handle;
        while (reg_iter &&  (reg_handle = vpi_scan(reg_iter))) {
          std::string regname = vpi_get_str(vpiName, reg_handle);
          std::string regpath = modname + "." + regname;
          size_t regid = !wire || wirename == regname ? 
            add_signal(new vpi_sig(reg_handle), regpath) : 0;
          id = regid ? regid : id;
          if (id > 0) break;
        }
        if (id > 0) break;

        // Iterate its mems
        vpiHandle mem_iter = vpi_iterate(vpiRegArray, mod_handle);
        vpiHandle mem_handle;
        while (mem_iter &&  (mem_handle = vpi_scan(mem_iter))) {
          std::string memname = vpi_get_str(vpiName, mem_handle);
          if (!wire || arrname == memname) {
            vpiHandle elm_iter = vpi_iterate(vpiReg, mem_handle);
            // size_t idx = vpi_get(vpiSize, mem_handle); FIXME what is idx?
            vpiHandle elm_handle;
            while (elm_iter &&  (elm_handle = vpi_scan(elm_iter))) {
              std::string elmname = vpi_get_str(vpiName, elm_handle);
              std::string elmpath = modname + "." + elmname;
              size_t elmid = add_signal(new vpi_sig(elm_handle), elmpath);
              id = wirename == elmname ? elmid : id;
            }
          }
          if (id > 0) break;
        }
      }

      // Find DFF
      if (!wire || wirepath == modname) {
#if defined(vpiPrimitive)
        vpiHandle udp_iter = vpi_iterate(vpiPrimitive, mod_handle);
        vpiHandle udp_handle;
        while (udp_iter &&  (udp_handle = vpi_scan(udp_iter))) {
           if (vpi_get(vpiPrimType, udp_handle) == vpiSeqPrim) {
             id = add_signal(new vpi_sig(udp_handle), modname);
             break;
          }
        }
#else
#warning "Simulator VPI does not support vpiPrimitive."
        std::cout << "Wire " << wire << " possibly belongs to a Primitive/UDP/DFF but simulator VPI has no support." << std::endl;
#endif
      }
      if (id > 0) break;

      vpiHandle sub_iter = vpi_iterate(vpiModule, mod_handle);
      vpiHandle sub_handle;
      while (sub_iter &&  (sub_handle = vpi_scan(sub_iter))) {
        modules.push(sub_handle);
      }
    }

    return id;
  }

  virtual int search(const std::string &wire) {
    return search_signals(wire.c_str());
  }
};

#endif // __VPI_H
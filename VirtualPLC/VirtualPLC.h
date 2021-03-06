#ifndef VIRTUALPLC_H
#define VIRTUALPLC_H

#include "Counter.h"
#include "Memory.h"
#include "MemoryConfig.h"
#include "Processor.h"
#include "Timer.h"
#include "PeriodicTask.h"

#include <atomic>
#include <array>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using counters_t = std::array<Counter, MemoryConfig::nCOUNTERS>;
using timers_t = std::array<Timer, MemoryConfig::nTIMERS>;

/// A VirtualPLC contains a processor, memory, timers and counters,
/// and is able to execute the program stored in memory.
/// Timers and counters are mapped to memory.
/// Based on Mitsubishi Melsec PLC: FX1N
class VirtualPLC
{
public:
   VirtualPLC();
   VirtualPLC(const VirtualPLC& other) = delete;
   VirtualPLC& operator=(const VirtualPLC& other) = delete;
   virtual ~VirtualPLC();
   /// Stop PLC and put PLC in initial state (all vars, timers and counters
   /// get initial value)
   void clear();
   bool readBinFile(const std::string& fileName);
   bool writeBinFile(const std::string& fileName);
   void loadMemory(const Memory& mem) noexcept;
   /// Put PLC in running mode
   void run();
   /// Test run status
   bool isRunning() const noexcept { return _isRunning; }
   /// PLC continues running
   void start();
   /// Halt PLC, stop running
   void stop();
   /// Get output by index (0..nY)
   bool getY(int32_t index) const noexcept {
     return _memory[MemoryConfig::OUTPUT_Y].integer & (1 << index); }
   /// Set input by index (0..nX) en status (true/false)
   void setX(int32_t index, bool status);
   int32_t getYs() const noexcept {
     return _memory[MemoryConfig::OUTPUT_Y].integer; }
   void setXs(int32_t status) noexcept {
     _memory[MemoryConfig::INPUT_X].integer = status; }
   int getMaxX() const noexcept { return nX; }
   int getMaxY() const noexcept { return nY; }
private:
   /// Number of inputs.
   static const int32_t nX = 8;
   /// Number of outputs.
   static const int32_t nY = 6;
   static const int32_t TICK = 100; // in msec
   static const int32_t RUN = 50; // in msec
   std::mutex _mutexIO;
   std::ifstream _plcBinFile;
   std::ofstream _plcBinFileOut;
   /// "Hardware"
   Memory _memory;
   timers_t _timers;
   counters_t _counters;
   Processor _processor;
   /// Stores memory status.
   bool _memoryIsLoaded;
   /// Stores running status.
   volatile std::atomic_bool _isRunning;
   /// Stores the number of loops executed.
   unsigned long long _step;
   PeriodicTask _tickTask;
   PeriodicTask _runTask;
   /// Updates all timers.
   void tick();
   void resetXs() noexcept { _memory[MemoryConfig::INPUT_X].integer = 0; }
   void resetYs() noexcept { _memory[MemoryConfig::OUTPUT_Y].integer = 0; }
};

#endif // VIRTUALPLC_H

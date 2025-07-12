#pragma once
#include "LabSound/LabSound.h"
#include "sigslot/signal.hpp"
#include "util.h"
#include "yaml-cpp/yaml.h"
#include <any>
#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using lab::FunctionNode;
using std::any;
using std::string;
using std::shared_ptr;
using std::unordered_map;
using std::vector;

// Building a command system that allows for undo/redo functionality
// Commands are actions that can be executed, undone, and redone.


class IReceiver {
public:
    IReceiver() {}
    virtual ~IReceiver() = default;

    // Execute the command with the given value
    virtual void doAction(any value, bool undo=false) = 0;
};


class Command {
public:
    Command(shared_ptr<IReceiver> receiver_, string id, any currentValue, any newValue)
        : receiver_(receiver_), _id(id), currentValue_(currentValue), newValue_(newValue) {
    }
    virtual ~Command() = default;
    string getId() const {
        return _id;
    }
    void setNewValue(any newValue) {
        newValue_ = newValue;
    }
    void setCurrentValue(any currentValue) {
        currentValue_ = currentValue;
    }
    any getCurrentValue() const {
        return currentValue_;
    }
    any getNewValue() const {
        return newValue_;
    }
  virtual void execute() { receiver_->doAction(newValue_, false); };
  virtual void undo() { receiver_-> doAction(currentValue_, true); };

protected:
    string _id;
    shared_ptr<IReceiver> receiver_;
    any currentValue_;
    any newValue_;
};

class CommandBatch {
public:
    CommandBatch(const string& batchId) : _batchId(batchId) {}
    
    void addCommand(shared_ptr<Command> command) {
        _commands.push_back(command);
    }
    
    void undo() {
        // Undo commands in reverse order
        for (auto it = _commands.rbegin(); it != _commands.rend(); ++it) {
            (*it)->undo();
        }
    }
    
    void redo() {
        // Redo commands in original order
        for (auto& command : _commands) {
            command->execute();
        }
    }
    
    string getId() const { return _batchId; }
    bool isEmpty() const { return _commands.empty(); }
    size_t size() const { return _commands.size(); }
    
private:
    string _batchId;
    vector<shared_ptr<Command>> _commands;
};

// TODO: Have debounce run in thread and not based on timing from audio callback
class UndoManager {
public:
    std::atomic<bool> hasFlushableCommands = false;
    shared_ptr<FunctionNode> undoNode;
    shared_ptr<lab::AudioContext> _audioContext;
    UndoManager(shared_ptr<lab::AudioContext> audioContext, float debounceTimeMs = 150.0f)
        :  _audioContext(audioContext), _batchingEnabled(false), _currentBatchId("")
    {
        _debounceTimeSamples = msToSamples(debounceTimeMs, audioContext->sampleRate());
        // Initialize the undo node
        undoNode = std::make_shared<FunctionNode>(*_audioContext.get());
        undoNode->start(0.0);
        undoNode->setFunction([this](lab::ContextRenderLock & r, lab::FunctionNode * me, int channel, float * buffer, int bufferSize) {
            // Process the audio block for the undo node
            processCallback(bufferSize, r.context()->sampleRate());
        });
        _audioContext->connect(_audioContext->destinationNode(), undoNode);
        _audioContext->synchronizeConnections();
    };
    ~UndoManager() = default;

    
    void executeCommand(shared_ptr<Command> command, bool debounceCommand=true) {
        command->execute();

        // If batching is enabled, add to current batch
        if (_batchingEnabled && _currentBatch) {
            _currentBatch->addCommand(command);
            return;
        }

        if(debounceCommand){
            // Add the command to the pending undo commands (normal behavior)
            auto &entry = _pendingUndoCommands[command->getId()];

            if (entry.command && entry.command->getId() == command->getId()) {
              // If the command already exists, just update it
              entry.command->setNewValue(command->getNewValue());
              entry.samplesSinceLastCommand = 0.0f;
              entry.readyToFlush = false;
              return;
            } else {
              // If the command is new, create a new entry
              entry.command = command;
              entry.readyToFlush = false;
              entry.samplesSinceLastCommand = 0.0f;
            }
        }
        else{
            _undoStack.push_back(command);
        }
        
        
        
    }

    void undo() {
        // First try to undo from batch stack
        if (!_batchUndoStack.empty()) {
            auto batch = _batchUndoStack.back();
            batch->undo();
            _batchRedoStack.push_back(batch);
            _batchUndoStack.pop_back();
        }
        // Then try individual commands
        else if (!_undoStack.empty()) {
            auto command = _undoStack.back();
            command->undo();
            _redoStack.push_back(command);
            _undoStack.pop_back();
        }
    }

    void redo() {
        if (!_redoStack.empty()) {
            auto command = _redoStack.back();
            command->execute();
            _undoStack.push_back(command);
            _redoStack.pop_back();
        } 
        else if (!_batchRedoStack.empty()) {
            auto batch = _batchRedoStack.back();
            batch->redo();
            _batchUndoStack.push_back(batch);
            _batchRedoStack.pop_back();
        }
    }

    // Batch management methods
    void startBatch(const string& batchId = "") {
        if (_batchingEnabled) {
            // End current batch before starting a new one
            endBatch();
        }
        
        _batchingEnabled = true;
        _currentBatchId = batchId.empty() ? generateBatchId() : batchId;
        _currentBatch = std::make_shared<CommandBatch>(_currentBatchId);
    }
    
    void endBatch() {
        if (!_batchingEnabled || !_currentBatch) {
            return;
        }
        
        _batchingEnabled = false;
        
        // Only add non-empty batches to the undo stack
        if (!_currentBatch->isEmpty()) {
            _batchUndoStack.push_back(_currentBatch);
            _batchRedoStack.clear(); // Clear redo stack on new batch
        }
        
        _currentBatch = nullptr;
        _currentBatchId = "";
    }
    
    bool isBatching() const {
        return _batchingEnabled;
    }
    
    string getCurrentBatchId() const {
        return _currentBatchId;
    }



    void processCallback(int bufferSize, float sampleRate) {
        if (bufferSize <= 0 || sampleRate <= 0.0)
            return;

        
        bool anyReady = false;

        for (auto &[id, entry] : _pendingUndoCommands) {
            if (!entry.command)
              continue;

            entry.samplesSinceLastCommand += bufferSize;

            if (entry.samplesSinceLastCommand >= _debounceTimeSamples &&
                !entry.readyToFlush) {
              entry.readyToFlush = true;
              anyReady = true;
            }
        }

        if (anyReady) {
            hasFlushableCommands.store(true, std::memory_order_release);
        }
    }

    void flushToUndoStack(){
        if (!hasFlushableCommands.load(std::memory_order_acquire))
            return;

        bool stillPending = false;

        for (auto it = _pendingUndoCommands.begin(); it != _pendingUndoCommands.end(); ) {
            auto& entry = it->second;
            if (entry.readyToFlush && entry.command) {
                _undoStack.push_back(std::move(entry.command));
                _redoStack.clear(); // Clear redo stack on new command
                it = _pendingUndoCommands.erase(it);
            } else {
                ++it;
                if (entry.command) stillPending = true;
            }
        }

        hasFlushableCommands.store(stillPending, std::memory_order_release);
    }
    
private:
    string generateBatchId() {
        static int batchCounter = 0;
        return "batch_" + std::to_string(++batchCounter);
    }
  float _debounceTimeSamples = 0.0f;
  bool _batchingEnabled = false;
  string _currentBatchId = "";
  shared_ptr<CommandBatch> _currentBatch = nullptr;
  
  struct PendingUndoCommand {
        shared_ptr<Command> command;
        float samplesSinceLastCommand = 0.0f;
        bool readyToFlush = false;
  };

  
  unordered_map<string, PendingUndoCommand> _pendingUndoCommands;
  vector<shared_ptr<Command>> _undoStack;
  vector<shared_ptr<CommandBatch>> _batchUndoStack;
  vector<shared_ptr<Command>> _redoStack;
  vector<shared_ptr<CommandBatch>> _batchRedoStack;
};


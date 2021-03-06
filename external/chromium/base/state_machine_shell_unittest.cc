// Copyright (c) 2014 Google Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/state_machine_shell.h"

#include <list>
#include <iostream>

#include "base/logging.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace hsm {

// Constant to represent no version for TestHsm::Expect.
static const uint64_t kNoVersion = static_cast<uint64_t>(-1);

// States for the test HSM
enum TestState {
  kStateS0,
  kStateS1,
  kStateS11,
  kStateS2,
  kStateS21,
  kStateS211,
  kStateT0,
};

// Events for the test HSM
enum TestEvent {
  kEventA,
  kEventB,
  kEventC,
  kEventD,
  kEventE,
  kEventF,
  kEventG,
  kEventH,
  kEventI,
  kEventJ,
};

// An enumeration of things that the HSM does that we can sense and then
// assert about.
enum HsmEvent {
  kHsmEnter,
  kHsmExit,
  kHsmHandled
};

} // namespace hsm

// --- Test Subclass ---

// StateMachineShell is an abstract class, so we must subclass it to test it.
// This class uses the sample test state machine specified by Miro Samek in his
// Practical Statecharts book. It covers the interesting transitions and
// topologies, so if it's fully exercised, it should represent a
// close-to-canonical test of the state machine's facilities.
//
// The diagram of this statechart is reproduced here:
// http://www.state-machine.com/resources/Heinzmann04.pdf
//
// This version has:
//  - A new event, I, in state S11, to test reentrant event handling.
//  - A new event, J, and new state T0, to test the no top state case.
class TestHsm : public base::StateMachineShell<hsm::TestState, hsm::TestEvent> {
 public:

  // --- Some types to aid sensing ---

  struct ResultEvent {
    // The state the HSM was in when it handled the event.
    const StateEnumN state;

    // The data passed into the event, if any.
    const void *data;

    // The state that actually handled the event (could be an ancestor of the
    // current state.
    const StateEnumN event_state;

    // The event that was handled.
    const EventEnumN event;

    // The "HSM Event" that occurred causing this to be recorded.
    const hsm::HsmEvent hsm_event;

    // The state version at the time the HsmEvent occured.
    const uint64_t version;
  };

  // --- Extra state to aid sensing ---

  // Foo is used as extended state and in guard conditions in the test HSM (see
  // link above).
  bool foo_;

  // A counter for how many times the S11 I handler is invoked.
  int event_i_count_;

  // A collection of interesting things that has happened to this state
  // machine. Used to validate that the HSM behaved as expected.
  std::list<ResultEvent> results;

  TestHsm()
      : base::StateMachineShell<hsm::TestState, hsm::TestEvent>("TestHsm"),
        foo_(true),
        event_i_count_(0) {
    EnableLogging();
  }
  virtual ~TestHsm() { }

  // Clears the results list, so nothing bleeds between test cases.
  void ClearResults() {
    results.clear();
  }

  // Consumes and validates a ResultEvent from the results list.
  void Expect(hsm::HsmEvent hsm_event,
              StateEnumN event_state = StateEnumN(),
              EventEnumN event = EventEnumN(),
              StateEnumN current_state = StateEnumN(),
              void *data = NULL,
              uint64_t version = hsm::kNoVersion) {
    DLOG(INFO) << __FUNCTION__ << ": hsm_event=" << hsm_event
               << ", event_state=" << GetStateString(event_state)
               << ", event=" << GetEventString(event)
               << ", current_state=" << GetStateString(current_state)
               << ", data=0x" << std::hex << data
               << ", version=" << version;
    EXPECT_FALSE(results.empty());
    TestHsm::ResultEvent result = results.front();
    results.pop_front();
    EXPECT_EQ(hsm_event, result.hsm_event);
    if (!event_state.is_null()) {
      EXPECT_EQ(event_state, result.event_state);
      if (current_state.is_null()) {
        EXPECT_EQ(event_state, result.state);
      }
    }

    if (!current_state.is_null()) {
      EXPECT_EQ(current_state, result.state);
    }

    if (!event.is_null()) {
      EXPECT_EQ(event, result.event);
    }
    EXPECT_EQ(data, result.data);

    if (version != hsm::kNoVersion) {
      EXPECT_EQ(version, result.version);
    }
  }


  // --- StateMachineShell Implementation ---
 protected:
  virtual StateEnumN GetUserParentState(hsm::TestState state) const OVERRIDE {
    switch (state) {
      case hsm::kStateS1:
      case hsm::kStateS2:
        return hsm::kStateS0;
      case hsm::kStateS11:
        return hsm::kStateS1;
      case hsm::kStateS21:
        return hsm::kStateS2;
      case hsm::kStateS211:
        return hsm::kStateS21;
      default:
        return StateEnumN::Null();
    }
  }

  virtual StateEnumN GetUserInitialSubstate(hsm::TestState state) const OVERRIDE {
    switch (state) {
      case hsm::kStateS0:
        return hsm::kStateS1;
      case hsm::kStateS1:
        return hsm::kStateS11;
      case hsm::kStateS2:
        return hsm::kStateS21;
      case hsm::kStateS21:
        return hsm::kStateS211;
      default:
        return StateEnumN::Null();
    }
  }

  virtual hsm::TestState GetUserInitialState() const OVERRIDE {
    return hsm::kStateS0;
  }

  virtual const char *GetUserStateString(hsm::TestState state) const OVERRIDE {
    switch (state) {
      case hsm::kStateS0:
        return "S0";
      case hsm::kStateS1:
        return "S1";
      case hsm::kStateS11:
        return "S11";
      case hsm::kStateS2:
        return "S2";
      case hsm::kStateS21:
        return "S21";
      case hsm::kStateS211:
        return "S211";
      case hsm::kStateT0:
        return "T0";
      default:
        return NULL;
    }
  }

  virtual const char *GetUserEventString(hsm::TestEvent event) const OVERRIDE {
    switch (event) {
      case hsm::kEventA:
        return "A";
      case hsm::kEventB:
        return "B";
      case hsm::kEventC:
        return "C";
      case hsm::kEventD:
        return "D";
      case hsm::kEventE:
        return "E";
      case hsm::kEventF:
        return "F";
      case hsm::kEventG:
        return "G";
      case hsm::kEventH:
        return "H";
      case hsm::kEventI:
        return "I";
      case hsm::kEventJ:
        return "J";
      default:
        return NULL;
    }
  }

  virtual Result HandleUserStateEvent(hsm::TestState state,
                                      hsm::TestEvent event,
                                      void *data) OVERRIDE {
    DLOG(INFO) << __FUNCTION__ << "(" << GetStateString(state) << ", "
               << GetEventString(event) << ", 0x" << std::hex << data << ");";

    Result result(kNotHandled);
    switch (state) {
      case hsm::kStateS0:
        switch (event) {
          case hsm::kEventE:
            result = hsm::kStateS211;
            break;
          case hsm::kEventJ:
            result = hsm::kStateT0;
            break;
          default:
            // Not Handled
            break;
        }
        break;

      case hsm::kStateS1:
        switch (event) {
          case hsm::kEventA:
            result = Result(hsm::kStateS1, true);
            break;
          case hsm::kEventB:
            result = hsm::kStateS11;
            break;
          case hsm::kEventC:
            result = hsm::kStateS2;
            break;
          case hsm::kEventD:
            result = hsm::kStateS0;
            break;
          case hsm::kEventF:
            result = hsm::kStateS211;
            break;
          default:
            // Not Handled
            break;
        }
        break;

      case hsm::kStateS11:
        switch (event) {
          case hsm::kEventG:
            result = hsm::kStateS211;
            break;
          case hsm::kEventH:
            if (foo_) {
              foo_ = false;
              result = kHandled;
              break;
            }
            break;
          case hsm::kEventI:
            // Inject another I every other time I is handled so every I should
            // ultimately increase event_i_count_ by 2.
            ++event_i_count_;
            if (event_i_count_ % 2 == 1) {
              // This should queue and be run before Handle() returns.
              Handle(hsm::kEventI);
              result = hsm::kStateS1;
              break;
            } else {
              result = kHandled;
              break;
            }
            break;
          default:
            // Not Handled
            break;
        }
        break;

      case hsm::kStateS2:
        switch (event) {
          case hsm::kEventC:
            result = hsm::kStateS1;
            break;
          case hsm::kEventF:
            result = hsm::kStateS11;
            break;
          default:
            // Not Handled
            break;
        }
        break;

      case hsm::kStateS21:
        switch (event) {
          case hsm::kEventB:
            result = hsm::kStateS211;
            break;
          case hsm::kEventH:
            if (!foo_) {
              foo_ = true;
              result = Result(hsm::kStateS21, true);
              break;
            }
            break;
          default:
            // Not Handled
            break;
        }
        break;

      case hsm::kStateS211:
        switch (event) {
          case hsm::kEventD:
            result = hsm::kStateS21;
            break;
          case hsm::kEventG:
            result = hsm::kStateS0;
            break;
          default:
            // Not Handled
            break;
        }
        break;

      case hsm::kStateT0:
        switch (event) {
          case hsm::kEventJ:
            result = hsm::kStateS0;
            break;
          default:
            // Not Handled
            break;
        }
        break;

      default:
        // Not Handled
        break;
    }

    if (result.is_handled) {
      AddEvent(state, event, data, hsm::kHsmHandled);
    }

    return result;
  }

  virtual void HandleUserStateEnter(hsm::TestState state) OVERRIDE {
    DLOG(INFO) << __FUNCTION__ << "(" << GetStateString(state) << ", ENTER);";
    AddEvent(state, EventEnumN::Null(), NULL, hsm::kHsmEnter);
  }

  virtual void HandleUserStateExit(hsm::TestState state) OVERRIDE {
    DLOG(INFO) << __FUNCTION__ << "(" << GetStateString(state) << ", EXIT);";
    AddEvent(state, EventEnumN::Null(), NULL, hsm::kHsmExit);
  }

 private:
  // Adds a new record to the result list.
  void AddEvent(hsm::TestState state,
                EventEnumN event,
                void *data,
                hsm::HsmEvent hsm_event) {
    ResultEvent result_event = { this->state(), data, state, event, hsm_event,
                                 this->version() };
    results.push_back(result_event);
  }
};


// --- Test Definitions ---

// This test validates that a state machine will initialize itself when it
// handles its first event, even if the user has not explicitly called
// initialize.
TEST(StateMachineShellTest, AutoInit) {
  TestHsm hsm;

  hsm.ClearResults();
  hsm.Handle(hsm::kEventA);

  // The HSM should Auto-Initialize
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS0);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS11);

  // Then it should handle the event
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS1, hsm::kEventA, hsm::kStateS11, 0);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS11);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS11);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(1, hsm.version());
  EXPECT_EQ(hsm::kStateS11, hsm.state());
}


// This test validates that if Handle() is called from within an event handler,
// it queues the event rather than trying to Handle the next event
// reentrantly. This behavior, or something like it, is required to make the
// state machine truly run-to-completion.
TEST(StateMachineShellTest, ReentrantHandle) {
  TestHsm hsm;
  hsm.Initialize();
  EXPECT_EQ(0, hsm.version());
  hsm.ClearResults();

  // Test a Handle() inside Handle()
  EXPECT_EQ(0, hsm.event_i_count_);
  hsm.Handle(hsm::kEventI);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS11, hsm::kEventI, hsm::kStateS11,
             NULL, 0);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS11);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS11);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS11, hsm::kEventI, hsm::kStateS11,
             NULL, 1);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(1, hsm.version());
  EXPECT_EQ(2, hsm.event_i_count_);
}

// This test validates that every meaningful event in every state in the test
// state machine behaves as expected. This should cover all normal operation of
// the state machine framework, except what is extracted into their own test
// cases above.
TEST(StateMachineShellTest, KitNKaboodle) {
  TestHsm hsm;

  // Test the initial state
  EXPECT_EQ(0, hsm.version());
  hsm.Initialize();
  EXPECT_EQ(0, hsm.version());
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS0);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS11);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS11, hsm.state());

  // Test IsIn
  EXPECT_TRUE(hsm.IsIn(hsm::kStateS11));
  EXPECT_TRUE(hsm.IsIn(hsm::kStateS1));
  EXPECT_TRUE(hsm.IsIn(hsm::kStateS0));
  EXPECT_FALSE(hsm.IsIn(hsm::kStateS2));
  EXPECT_FALSE(hsm.IsIn(hsm::kStateS21));
  EXPECT_FALSE(hsm.IsIn(hsm::kStateS211));

  // State: S11, Event: A
  hsm.ClearResults();
  hsm.Handle(hsm::kEventA);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS1, hsm::kEventA, hsm::kStateS11, 0);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS11);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS11);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(1, hsm.version());
  EXPECT_EQ(hsm::kStateS11, hsm.state());

  // State: S11, Event: B
  hsm.ClearResults();
  hsm.Handle(hsm::kEventB);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS1, hsm::kEventB, hsm::kStateS11,
             NULL, 1);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS11);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS11);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(2, hsm.version());
  EXPECT_EQ(hsm::kStateS11, hsm.state());

  // State: S11, Event: D
  hsm.ClearResults();
  hsm.Handle(hsm::kEventD);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS1, hsm::kEventD, hsm::kStateS11,
             NULL, 2);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS11);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS11);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(3, hsm.version());
  EXPECT_EQ(hsm::kStateS11, hsm.state());

  // State: S11, Event: H (foo == true)
  hsm.ClearResults();
  EXPECT_TRUE(hsm.foo_);
  hsm.Handle(hsm::kEventH);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS11, hsm::kEventH, hsm::kStateS11,
             NULL, 3);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(3, hsm.version());
  EXPECT_EQ(hsm::kStateS11, hsm.state());
  EXPECT_FALSE(hsm.foo_);

  // State: S11, Event: H (foo == false)
  hsm.ClearResults();
  hsm.Handle(hsm::kEventH);
  EXPECT_FALSE(hsm.foo_);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(3, hsm.version());
  EXPECT_EQ(hsm::kStateS11, hsm.state());

  // State: S11, Event: C
  hsm.ClearResults();
  hsm.Handle(hsm::kEventC);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS1, hsm::kEventC, hsm::kStateS11,
             NULL, 3);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS11);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS2);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS21);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS211);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(4, hsm.version());
  EXPECT_EQ(hsm::kStateS211, hsm.state());

  // State: S211, Event: A
  hsm.ClearResults();
  hsm.Handle(hsm::kEventA);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS211, hsm.state());

  // State: S211, Event: B
  hsm.ClearResults();
  hsm.Handle(hsm::kEventB);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS21, hsm::kEventB, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS211);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS211);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS211, hsm.state());

  // State: S211, Event: D
  hsm.ClearResults();
  hsm.Handle(hsm::kEventD);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS211, hsm::kEventD, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS211);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS211);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS211, hsm.state());

  // State: S211, Event: E
  hsm.ClearResults();
  hsm.Handle(hsm::kEventE);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS0, hsm::kEventE, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS21);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS2);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS2);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS21);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS211);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS211, hsm.state());

  // State: S211, Event: F
  hsm.ClearResults();
  hsm.Handle(hsm::kEventF);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS2, hsm::kEventF, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS21);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS2);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS11);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS11, hsm.state());

  // State: S11, Event: E
  hsm.ClearResults();
  hsm.Handle(hsm::kEventE);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS0, hsm::kEventE, hsm::kStateS11);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS11);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS2);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS21);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS211);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS211, hsm.state());

  // State: S211, Event: G
  hsm.ClearResults();
  hsm.Handle(hsm::kEventG);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS211, hsm::kEventG, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS21);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS2);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS11);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS11, hsm.state());

  // State: S11, Event: F
  hsm.ClearResults();
  hsm.Handle(hsm::kEventF);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS1, hsm::kEventF, hsm::kStateS11);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS11);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS2);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS21);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS211);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS211, hsm.state());

  // State: S211, Event: H (foo == false)
  EXPECT_FALSE(hsm.foo_);
  hsm.ClearResults();
  hsm.Handle(hsm::kEventH);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS21, hsm::kEventH, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS21);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS21);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS211);
  EXPECT_TRUE(hsm.foo_);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS211, hsm.state());

  // State: S211, Event: H (foo == true)
  hsm.ClearResults();
  hsm.Handle(hsm::kEventH);
  EXPECT_TRUE(hsm.foo_);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS211, hsm.state());

  // State: S211, Event: C
  hsm.ClearResults();
  hsm.Handle(hsm::kEventC);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS2, hsm::kEventC, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS21);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS2);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS11);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS11, hsm.state());

  // State: S11, Event: G
  hsm.ClearResults();
  hsm.Handle(hsm::kEventG);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS11, hsm::kEventG, hsm::kStateS11);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS11);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS2);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS21);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS211);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS211, hsm.state());

  // State: S211, Event: J
  hsm.ClearResults();
  hsm.Handle(hsm::kEventJ);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS0, hsm::kEventJ, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS211);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS21);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS2);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS0);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateT0);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateT0, hsm.state());

  // State: T0, Event: J
  hsm.ClearResults();
  hsm.Handle(hsm::kEventJ);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateT0, hsm::kEventJ);
  hsm.Expect(hsm::kHsmExit, hsm::kStateT0);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS0);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS11);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS11, hsm.state());

  // Test that event data gets passed through to the handler
  hsm.ClearResults();
  void *data = reinterpret_cast<void *>(7);
  hsm.Handle(hsm::kEventC, data);
  hsm.Expect(hsm::kHsmHandled, hsm::kStateS1, hsm::kEventC, hsm::kStateS11,
             data);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS11);
  hsm.Expect(hsm::kHsmExit, hsm::kStateS1);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS2);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS21);
  hsm.Expect(hsm::kHsmEnter, hsm::kStateS211);
  EXPECT_EQ(0, hsm.results.size());
  EXPECT_EQ(hsm::kStateS211, hsm.state());
}

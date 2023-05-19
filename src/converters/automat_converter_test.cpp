#include "automat_converter.hpp"
#include <userver/formats/json/value_builder.hpp>

#include <userver/utest/utest.hpp>

UTEST(Signal, Circle) {
  formats::json::ValueBuilder builder;
  Signal original = Signal("something");
  builder = original;
  Signal deserialized_signal = builder.ExtractValue().As<Signal>();
  ASSERT_EQ(deserialized_signal, original);
}

UTEST(Signal, Empty) {
  formats::json::ValueBuilder builder;
  Signal original = Signal();
  builder = original;
  Signal deserialized_signal = builder.ExtractValue().As<Signal>();
  ASSERT_EQ(deserialized_signal, original);
}

UTEST(Signal, Roots) {
  formats::json::ValueBuilder builder;
  Signal original = Signal("id1_id2", {Signal("id1"), Signal("id2")});
  builder = original;
  Signal deserialized_signal = builder.ExtractValue().As<Signal>();
  ASSERT_EQ(deserialized_signal.id, original.id);
  ASSERT_NE(deserialized_signal.source_signals, original.source_signals);
  ASSERT_NE(deserialized_signal, original);
}
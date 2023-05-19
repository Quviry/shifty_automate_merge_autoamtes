#pragma once

#include <userver/formats/json/value.hpp>

#include "../models/automat.hpp"

using namespace userver;

Signal Parse(const formats::json::Value& json, formats::parse::To<Signal>);

State Parse(const formats::json::Value& json, formats::parse::To<State>);

Automat Parse(const formats::json::Value& json, formats::parse::To<Automat>);

formats::json::Value Serialize(const State& state,
                               formats::serialize::To<formats::json::Value>);

formats::json::Value Serialize(const Signal& signal,
                               formats::serialize::To<formats::json::Value>);

formats::json::Value Serialize(const Automat& data,
                               formats::serialize::To<formats::json::Value>);
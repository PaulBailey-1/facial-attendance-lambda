#pragma once

#include <array>
#include <boost/core/span.hpp>

#define FACE_VEC_SIZE 128

struct Update {

	int id;
	int device_id;
	std::array<float, FACE_VEC_SIZE> facial_features;

	Update(int id_, int device_id_, boost::span<const UCHAR> facial_features_) {
		id = id_;
		device_id = device_id_;
		memcpy(facial_features.data(), facial_features_.data(), facial_features_.size_bytes());
	}
};

struct LongTermState {

	int id;
	std::array<float, FACE_VEC_SIZE> facial_features;

	LongTermState(int id_, boost::span<const UCHAR> facial_features_) {
		id = id_;
		memcpy(facial_features.data(), facial_features_.data(), facial_features_.size_bytes());
	}
};
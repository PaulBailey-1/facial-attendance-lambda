#pragma once

#include <array>
#include <boost/core/span.hpp>

struct Update {

	int id;
	int device_id;
	std::array<float, 128> facial_features;

	Update(int id_, int device_id_, boost::span<const UCHAR> facial_features_) {
		id = id_;
		device_id = device_id_;
		memcpy(facial_features.data(), facial_features_.data(), facial_features_.size_bytes());
	}

};

#pragma once

namespace {

   static inline int32_t getTypeSizeInBytes(uint32_t ty) {
      if (ty == TINYGLTF_TYPE_SCALAR) {
         return 1;
      } else if (ty == TINYGLTF_TYPE_VEC2) {
         return 2;
      } else if (ty == TINYGLTF_TYPE_VEC3) {
         return 3;
      } else if (ty == TINYGLTF_TYPE_VEC4) {
         return 4;
      } else if (ty == TINYGLTF_TYPE_MAT2) {
         return 4;
      } else if (ty == TINYGLTF_TYPE_MAT3) {
         return 9;
      } else if (ty == TINYGLTF_TYPE_MAT4) {
         return 16;
      } else {
         // Unknown componenty type
         return -1;
      }
   }

   template <typename _VectorType>
   bool fixupNames(_VectorType& data, const char* pretty_name, const char* prefix_name) {
      ozz::set<std::string> names;
      for (size_t i = 0; i < data.size(); ++i) {
         bool renamed = false;
         typename _VectorType::const_reference _data = data[i];

         std::string name(_data.name.c_str());

         // Fixes unnamed animations.
         if (name.length() == 0) {
            renamed = true;
            name = prefix_name;
            name += std::to_string(i);
         }

         // Fixes duplicated names, while it has duplicates
         for (auto it = names.find(name); it != names.end(); it = names.find(name)) {
            renamed = true;
            name += "_";
            name += std::to_string(i);
         }

         // Update names index.
         if (!names.insert(name).second) {
            assert(false && "Algorithm must ensure no duplicated animation names.");
         }

         if (renamed) {
            Log::debug << pretty_name << " #" << i << " with name \"" << _data.name
                       << "\" was renamed to \"" << name << "\" in order to avoid duplicates."
                       << std::endl;

            // Actually renames tinygltf data.
            data[i].name = name;
         }
      }

      return true;
   }

   /// Returns the address of a gltf buffer given an accessor.
   /// Performs basic checks to ensure the data is in the correct format
   template <typename T>
   ozz::span<const T> bufferView(const tinygltf::Model& model, const tinygltf::Accessor& accessor) {
      const int32_t component_size = tinygltf::GetComponentSizeInBytes(accessor.componentType);
      const int32_t element_size = component_size * getTypeSizeInBytes(accessor.type);
      if (element_size != sizeof(T)) {
         Log::error << "Invalid buffer view access. Expected element size '" << sizeof(T) << " got "
                    << element_size << " instead." << std::endl;
         return ozz::span<const T>();
      }

      const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
      const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
      const T* begin = reinterpret_cast<const T*>(buffer.data.data() + bufferView.byteOffset +
                                                  accessor.byteOffset);
      return ozz::span<const T>(begin, accessor.count);
   }

   /// Samples a linear animation channel
   /// There is an exact mapping between gltf and ozz keyframes so we just copy
   /// everything over.
   template <typename _KeyframesType>
   bool sampleLinearChannel(const tinygltf::Model& model,
                            const tinygltf::Accessor& output,
                            const ozz::span<const float>& timestamps,
                            _KeyframesType* keyframes) {
      const size_t gltf_keys_count = output.count;

      if (gltf_keys_count == 0) {
         keyframes->clear();
         return true;
      }

      typedef typename _KeyframesType::value_type::Value ValueType;
      const ozz::span<const ValueType> values = bufferView<ValueType>(model, output);
      if (values.size_bytes() / sizeof(ValueType) != gltf_keys_count ||
          timestamps.size() != gltf_keys_count) {
         Log::error << "gltf format error, inconsistent number of keys." << std::endl;
         return false;
      }

      keyframes->reserve(output.count);
      for (size_t i = 0; i < output.count; ++i) {
         const typename _KeyframesType::value_type key{timestamps[i], values[i]};
         keyframes->push_back(key);
      }

      return true;
   }

   /// Samples a step animation channel
   /// There are twice-1 as many ozz keyframes as gltf keyframes
   template <typename _KeyframesType>
   bool sampleStepChannel(const tinygltf::Model& _model,
                          const tinygltf::Accessor& _output,
                          const ozz::span<const float>& _timestamps,
                          _KeyframesType* _keyframes) {
      const size_t gltf_keys_count = _output.count;

      if (gltf_keys_count == 0) {
         _keyframes->clear();
         return true;
      }

      typedef typename _KeyframesType::value_type::Value ValueType;
      const ozz::span<const ValueType> values = bufferView<ValueType>(_model, _output);
      if (values.size_bytes() / sizeof(ValueType) != gltf_keys_count ||
          _timestamps.size() != gltf_keys_count) {
         Log::error << "gltf format error, inconsistent number of keys." << std::endl;
         return false;
      }

      // A step is created with 2 consecutive keys. Last step is a single key.
      size_t numKeyframes = gltf_keys_count * 2 - 1;
      _keyframes->resize(numKeyframes);

      for (size_t i = 0; i < _output.count; i++) {
         typename _KeyframesType::reference key = _keyframes->at(i * 2);
         key.time = _timestamps[i];
         key.value = values[i];

         if (i < _output.count - 1) {
            typename _KeyframesType::reference next_key = _keyframes->at(i * 2 + 1);
            next_key.time = nexttowardf(_timestamps[i + 1], 0.f);
            next_key.value = values[i];
         }
      }

      return true;
   }

   /// Samples a hermite spline in the form
   /// p(t) = (2t^3 - 3t^2 + 1)p0 + (t^3 - 2t^2 + t)m0 + (-2t^3 + 3t^2)p1 + (t^3 -
   /// t^2)m1 where t is a value between 0 and 1 p0 is the starting point at t = 0
   /// m0 is the scaled starting tangent at t = 0
   /// p1 is the ending point at t = 1
   /// m1 is the scaled ending tangent at t = 1
   /// p(t) is the resulting point value
   template <typename T>
   T sampleHermiteSpline(float alpha, const T& p0, const T& m0, const T& p1, const T& m1) {
      assert(alpha >= 0.f && alpha <= 1.f);

      const float t1 = alpha;
      const float t2 = alpha * alpha;
      const float t3 = t2 * alpha;

      // a = 2t^3 - 3t^2 + 1
      const float a = 2.0f * t3 - 3.0f * t2 + 1.0f;
      // b = t^3 - 2t^2 + t
      const float b = t3 - 2.0f * t2 + t1;
      // c = -2t^3 + 3t^2
      const float c = -2.0f * t3 + 3.0f * t2;
      // d = t^3 - t^2
      const float d = t3 - t2;

      // p(t) = a * p0 + b * m0 + c * p1 + d * m1
      T pt = p0 * a + m0 * b + p1 * c + m1 * d;
      return pt;
   }

   /// Samples a cubic-spline channel
   /// the number of keyframes is determined from the animation duration and given
   /// sample rate
   template <typename _KeyframesType>
   bool sampleCubicSplineChannel(const tinygltf::Model& model,
                                 const tinygltf::Accessor& output,
                                 const ozz::span<const float>& timestamps,
                                 float sampling_rate,
                                 float duration,
                                 _KeyframesType* keyframes) {
      (void)duration;

      assert(output.count % 3 == 0);
      size_t gltf_keys_count = output.count / 3;

      if (gltf_keys_count == 0) {
         keyframes->clear();
         return true;
      }

      typedef typename _KeyframesType::value_type::Value ValueType;
      const ozz::span<const ValueType> values = bufferView<ValueType>(model, output);
      if (values.size_bytes() / (sizeof(ValueType) * 3) != gltf_keys_count ||
          timestamps.size() != gltf_keys_count) {
         Log::error << "gltf format error, inconsistent number of keys." << std::endl;
         return false;
      }

      // Iterate keyframes at _sampling_rate steps, between first and last time
      // stamps.
      ozz::animation::offline::FixedRateSamplingTime fixed_it(timestamps[gltf_keys_count - 1] -
                                                                  timestamps[0],
                                                              sampling_rate);
      keyframes->resize(fixed_it.num_keys());
      size_t cubic_key0 = 0;
      for (size_t k = 0; k < fixed_it.num_keys(); ++k) {
         const float time = fixed_it.time(k) + timestamps[0];

         // Creates output key.
         typename _KeyframesType::value_type key;
         key.time = time;

         // Makes sure time is in between the correct cubic keyframes.
         while (timestamps[cubic_key0 + 1] < time) {
            cubic_key0++;
         }
         assert(timestamps[cubic_key0] <= time && time <= timestamps[cubic_key0 + 1]);

         // Interpolate cubic key
         const float t0 = timestamps[cubic_key0];     // keyframe before time
         const float t1 = timestamps[cubic_key0 + 1]; // keyframe after time
         const float alpha = (time - t0) / (t1 - t0);
         const ValueType& p0 = values[cubic_key0 * 3 + 1];
         const ValueType m0 = values[cubic_key0 * 3 + 2] * (t1 - t0);
         const ValueType& p1 = values[(cubic_key0 + 1) * 3 + 1];
         const ValueType m1 = values[(cubic_key0 + 1) * 3] * (t1 - t0);
         key.value = sampleHermiteSpline(alpha, p0, m0, p1, m1);

         // Pushes interpolated key.
         keyframes->at(k) = key;
      }

      return true;
   }

   template <typename _KeyframesType>
   bool sampleChannel(const tinygltf::Model& model,
                      const std::string& interpolation,
                      const tinygltf::Accessor& output,
                      const ozz::span<const float>& timestamps,
                      float sampling_rate,
                      float duration,
                      _KeyframesType* keyframes) {
      bool valid = false;
      if (interpolation == "LINEAR") {
         valid = sampleLinearChannel(model, output, timestamps, keyframes);
      } else if (interpolation == "STEP") {
         valid = sampleStepChannel(model, output, timestamps, keyframes);
      } else if (interpolation == "CUBICSPLINE") {
         valid = sampleCubicSplineChannel(model,
                                          output,
                                          timestamps,
                                          sampling_rate,
                                          duration,
                                          keyframes);
      } else {
         Log::error << "Invalid or unknown interpolation type '" << interpolation << "'."
                    << std::endl;
         valid = false;
      }

      // Check if sorted (increasing time, might not be stricly increasing).
      if (valid) {
         valid = std::is_sorted(
             keyframes->begin(),
             keyframes->end(),
             [](typename _KeyframesType::const_reference _a,
                typename _KeyframesType::const_reference _b) { return _a.time < _b.time; });
         if (!valid) {
            Log::error << "gltf format error, keyframes are not sorted in increasing order."
                       << std::endl;
         }
      }

      // Remove keyframes with strictly equal times, keeping the first one.
      if (valid) {
         auto new_end = std::unique(
             keyframes->begin(),
             keyframes->end(),
             [](typename _KeyframesType::const_reference _a,
                typename _KeyframesType::const_reference _b) { return _a.time == _b.time; });
         if (new_end != keyframes->end()) {
            keyframes->erase(new_end, keyframes->end());

            Log::error << "gltf format error, keyframe times are not unique. "
                          "Imported data were modified to remove keyframes at "
                          "consecutive equivalent times."
                       << std::endl;
         }
      }
      return valid;
   }

   auto createTranslationRestPoseKey(const tinygltf::Node& node) {
      ozz::animation::offline::RawAnimation::TranslationKey key;
      key.time = 0.0f;

      if (node.translation.empty()) {
         key.value = ozz::math::Float3::zero();
      } else {
         key.value = ozz::math::Float3(static_cast<float>(node.translation[0]),
                                       static_cast<float>(node.translation[1]),
                                       static_cast<float>(node.translation[2]));
      }

      return key;
   }

   auto createRotationRestPoseKey(const tinygltf::Node& node) {
      ozz::animation::offline::RawAnimation::RotationKey key;
      key.time = 0.0f;

      if (node.rotation.empty()) {
         key.value = ozz::math::Quaternion::identity();
      } else {
         key.value = ozz::math::Quaternion(static_cast<float>(node.rotation[0]),
                                           static_cast<float>(node.rotation[1]),
                                           static_cast<float>(node.rotation[2]),
                                           static_cast<float>(node.rotation[3]));
      }
      return key;
   }

   auto createScaleRestPoseKey(const tinygltf::Node& node) {
      ozz::animation::offline::RawAnimation::ScaleKey key;
      key.time = 0.0f;

      if (node.scale.empty()) {
         key.value = ozz::math::Float3::one();
      } else {
         key.value = ozz::math::Float3(static_cast<float>(node.scale[0]),
                                       static_cast<float>(node.scale[1]),
                                       static_cast<float>(node.scale[2]));
      }
      return key;
   }

   // Creates the default transform for a gltf node
   bool createNodeTransform(const tinygltf::Node& node, ozz::math::Transform* transform) {
      *transform = ozz::math::Transform::identity();

      if (!node.matrix.empty()) {
         const ozz::math::Float4x4 matrix = {
             {ozz::math::simd_float4::Load(static_cast<float>(node.matrix[0]),
                                           static_cast<float>(node.matrix[1]),
                                           static_cast<float>(node.matrix[2]),
                                           static_cast<float>(node.matrix[3])),
              ozz::math::simd_float4::Load(static_cast<float>(node.matrix[4]),
                                           static_cast<float>(node.matrix[5]),
                                           static_cast<float>(node.matrix[6]),
                                           static_cast<float>(node.matrix[7])),
              ozz::math::simd_float4::Load(static_cast<float>(node.matrix[8]),
                                           static_cast<float>(node.matrix[9]),
                                           static_cast<float>(node.matrix[10]),
                                           static_cast<float>(node.matrix[11])),
              ozz::math::simd_float4::Load(static_cast<float>(node.matrix[12]),
                                           static_cast<float>(node.matrix[13]),
                                           static_cast<float>(node.matrix[14]),
                                           static_cast<float>(node.matrix[15]))}};
         ozz::math::SimdFloat4 translation, rotation, scale;
         if (ToAffine(matrix, &translation, &rotation, &scale)) {
            ozz::math::Store3PtrU(translation, &transform->translation.x);
            ozz::math::StorePtrU(rotation, &transform->rotation.x);
            ozz::math::Store3PtrU(scale, &transform->scale.x);
            return true;
         }

         Log::error << "Failed to extract transformation from node \"" << node.name << "\"."
                    << std::endl;
         return false;
      }

      if (!node.translation.empty()) {
         transform->translation = ozz::math::Float3(static_cast<float>(node.translation[0]),
                                                    static_cast<float>(node.translation[1]),
                                                    static_cast<float>(node.translation[2]));
      }
      if (!node.rotation.empty()) {
         transform->rotation = ozz::math::Quaternion(static_cast<float>(node.rotation[0]),
                                                     static_cast<float>(node.rotation[1]),
                                                     static_cast<float>(node.rotation[2]),
                                                     static_cast<float>(node.rotation[3]));
      }
      if (!node.scale.empty()) {
         transform->scale = ozz::math::Float3(static_cast<float>(node.scale[0]),
                                              static_cast<float>(node.scale[1]),
                                              static_cast<float>(node.scale[2]));
      }

      return true;
   }

}

#include "vision_service.hpp"

#include <longinus/retina_net.hpp>
#include <irisviel/face_service.hpp>
#include <romancia/face_alignment.hpp>
#include <gaius/feature_extractor.hpp>
#include <cassius/feature_extractor.hpp>

using namespace glasssix::gaius;
using namespace glasssix::cassius;
using namespace glasssix::romancia;
using namespace glasssix::irisviel;
using namespace glasssix::longinus;

namespace glasssix::exposing::nessus
{
	class vision_service::impl
	{
	public:
		impl()
		{
			// New
			functions_.insert_or_assign(u8"gaius.new", std::bind(&impl::gaius_new, this, std::placeholders::_1));
			functions_.insert_or_assign(u8"cassius.new", std::bind(&impl::cassius_new, this, std::placeholders::_1));
			functions_.insert_or_assign(u8"longinus.new", std::bind(&impl::longinus_new, this, std::placeholders::_1));
			functions_.insert_or_assign(u8"romancia.new", std::bind(&impl::romancia_new, this, std::placeholders::_1));
			functions_.insert_or_assign(u8"irisviel.new", std::bind(&impl::irisviel_new, this, std::placeholders::_1));

			// Delete
			functions_.insert_or_assign(u8"gaius.delete", meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"cassius.delete", meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"longinus.delete", meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"irisviel.delete", meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"romancia.delete", meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));

			// Business
			functions_.insert_or_assign(u8"longinus.detect", std::bind(&impl::longinus_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(u8"romancia.alignFace", std::bind(&impl::romancia_align_face, this, std::placeholders::_1));
			functions_.insert_or_assign(u8"gaius.Forward", std::bind(&impl::gaius_extract_feature, this, std::placeholders::_1));
			functions_.insert_or_assign(u8"cassius.Forward", std::bind(&impl::cassius_extract_feature, this, std::placeholders::_1));
			functions_.insert_or_assign(u8"irisviel.clear", meta::replace_return<unknown_object>(std::bind(&impl::irisviel_clear, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"irisviel.remove_all", meta::replace_return<unknown_object>(std::bind(&impl::irisviel_remove_all, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"irisviel.load_databases", meta::replace_return<unknown_object>(std::bind(&impl::irisviel_load_databases, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"irisviel.add_record", meta::replace_return<unknown_object>(std::bind(&impl::irisviel_add_record, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"irisviel.add_records", meta::replace_return<unknown_object>(std::bind(&impl::irisviel_add_records, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"irisviel.update_record", meta::replace_return<unknown_object>(std::bind(&impl::irisviel_update_record, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"irisviel.update_records", meta::replace_return<unknown_object>(std::bind(&impl::irisviel_update_records, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"irisviel.remove_record", meta::replace_return<unknown_object>(std::bind(&impl::irisviel_remove_record, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"irisviel.remove_records", meta::replace_return<unknown_object>(std::bind(&impl::irisviel_remove_records, this, std::placeholders::_1)));
			functions_.insert_or_assign(u8"irisviel.search", std::bind(&impl::irisviel_search, this, std::placeholders::_1));
		}

		param_string name() const
		{
			return u8"Glasssix Vision Service";
		}

		param_string version() const
		{
			return u8"1.0.0";
		}

		param_vector<param_string> get_available_functions() const
		{
			auto result = make_param_vector<param_string>();

			for (auto [key, value] : functions_)
			{
				result.push_back(key);
			}

			return result;
		}

		unknown_object execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const
		{
			auto iter = functions_.find(function_name);

			return iter != functions_.end() ? iter->second(params) : throw abi_key_not_found{ function_name };
		}
	private:
		unknown_object cassius_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<int>(params.get_value(u8"device"));

			return add_instance(make_exported_interface<cassius::feature_extractor>(u8"models/unicorn.phai", device));
		}

		unknown_object gaius_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<int>(params.get_value(u8"device"));

			return add_instance(make_exported_interface<gaius::feature_extractor>(u8"models/mobile_unicorn.phai", device));
		}

		unknown_object longinus_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto nms = unbox<float>(params.get_value(u8"nms"));

			return add_instance(make_exported_interface<retina_net>(u8"models/retina.phai", u8"models/retina.racy", nms, device));
		}

		unknown_object romancia_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));

			return add_instance(make_exported_interface<face_alignment>(device));
		}

		unknown_object irisviel_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto single_database_capacity = unbox<std::int32_t>(params.get_value(u8"single_database_capacity"));
			auto dimension = unbox<std::int32_t>(params.get_value(u8"dimension"));
			auto working_directory = unbox<param_string>(params.get_value(u8"working_directory"));

			return add_instance(make_exported_interface<face_service>(single_database_capacity, dimension, working_directory));
		}

		unknown_object cassius_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<cassius::feature_extractor>(params);
			auto aligned_faces = unbox<param_span<std::uint8_t>>(params.get_value(u8"aligned_faces"));
			auto num = unbox<std::int32_t>(params.get_value(u8"num"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.get(aligned_faces, num, order);
		}

		unknown_object gaius_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<gaius::feature_extractor>(params);
			auto aligned_faces = unbox<param_span<std::uint8_t>>(params.get_value(u8"aligned_faces"));
			auto num = unbox<std::int32_t>(params.get_value(u8"num"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.get(aligned_faces, num, order);
		}

		unknown_object longinus_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<retina_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto min_win = unbox<std::int32_t>(params.get_value(u8"min_win"));
			auto threshold = unbox<float>(params.get_value(u8"threshold"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.get(image, channels, height, width, min_win, threshold, order);
		}

		unknown_object romancia_align_face(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_alignment>(params);
			auto gray = unbox<param_span<std::uint8_t>>(params.get_value(u8"gray"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto faces = params.get_value(u8"faces").as<param_vector<face_info>>();

			return instance.get(gray, height, width, faces);
		}

		void irisviel_clear(const param_hash_map<param_string, unknown_object>& params)
		{
			get_instance<face_service>(params).clear();
		}

		void irisviel_remove_all(const param_hash_map<param_string, unknown_object>& params)
		{
			get_instance<face_service>(params).remove_all();
		}

		void irisviel_load_databases(const param_hash_map<param_string, unknown_object>& params)
		{
			get_instance<face_service>(params).load_databases();
		}

		void irisviel_add_record(const param_hash_map<param_string, unknown_object>& params)
		{
			irisviel_add_or_update_record_helper(params, false);
		}

		void irisviel_add_records(const param_hash_map<param_string, unknown_object>& params)
		{
			irisviel_add_or_update_records_helper(params, false);
		}

		void irisviel_update_record(const param_hash_map<param_string, unknown_object>& params)
		{
			irisviel_add_or_update_record_helper(params, true);
		}

		void irisviel_update_records(const param_hash_map<param_string, unknown_object>& params)
		{
			irisviel_add_or_update_records_helper(params, true);
		}

		void irisviel_remove_record(const param_hash_map<param_string, unknown_object>& params)
		{
			auto key = unbox<param_string>(params.get_value(u8"key"));

			get_instance<face_service>(params).remove_record(key);
		}

		void irisviel_remove_records(const param_hash_map<param_string, unknown_object>& params)
		{
			auto keys = params.get_value(u8"keys").as<param_vector<param_string>>();

			get_instance<face_service>(params).remove_records(keys);
		}

		unknown_object irisviel_search(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_service>(params);
			auto feature = params.get_value(u8"feature").as<param_vector<float>>();
			auto top = unbox<std::int32_t>(params.get_value(u8"top"));

			return instance.search(feature, top);
		}

		record irisviel_create_record_helper(const param_hash_map<param_string, unknown_object>& params)
		{
			auto dimension = unbox<std::int32_t>(params.get_value(u8"dimension"));
			auto key = unbox<param_string>(params.get_value(u8"key"));
			auto feature = params.get_value(u8"feature").as<param_vector<float>>();
			auto result = make_exported_interface<record>(dimension);

			result.key(key);
			result.feature(feature);

			return result;
		}

		void irisviel_add_or_update_record_helper(const param_hash_map<param_string, unknown_object>& params, bool update)
		{
			auto instance = get_instance<face_service>(params);
			auto record = irisviel_create_record_helper(params);

			if (update)
			{
				instance.add_record(record);
			}
			else
			{
				instance.update_record(record);
			}
		}

		void irisviel_add_or_update_records_helper(const param_hash_map<param_string, unknown_object>& params, bool update)
		{
			auto instance = get_instance<face_service>(params);
			auto param_records = params.get_value(u8"records").as<param_vector<param_hash_map<param_string, unknown_object>>>();
			auto records = make_param_vector<record>();

			for (const auto& item : param_records)
			{
				records.push_back(irisviel_create_record_helper(item));
			}

			if (update)
			{
				instance.add_records(records);
			}
			else
			{
				instance.update_records(records);
			}
		}

		unknown_object add_instance(const unknown_object& instance)
		{
			std::scoped_lock lock{ mutex_ };

			return nullptr;
		}

		void delete_instance_by_id(const guid& id)
		{
			std::scoped_lock lock{ mutex_ };

			if (auto iter = instances_.find(id); iter != instances_.end())
			{
				instances_.erase(iter);
			}
		}

		void delete_instance(const param_hash_map<param_string, unknown_object>& params)
		{
			delete_instance_by_id(get_instance_id(params));
		}

		static guid get_instance_id(const param_hash_map<param_string, unknown_object>& params)
		{
			return unbox<guid>(params.get_value(u8"object_id"));
		}

		template<typename T>
		T get_instance(const guid& id)
		{
			std::scoped_lock lock{ mutex_ };
			auto iter = instances_.find(id);

			return iter != instances_.end() ? iter->second.as<T>() : throw abi_key_not_found{ exposing::format(u8"Cannot find instance: {}.", to_param_string(id)) };
		}

		template<typename T>
		T get_instance(const param_hash_map<param_string, unknown_object>& params)
		{
			return get_instance<T>(unbox<guid>(params.get_value(u8"object_id")));
		}

		std::mutex mutex_;
		std::unordered_map<guid, unknown_object> instances_;
		std::unordered_map<param_string, std::function<unknown_object(const param_hash_map<param_string, unknown_object>&)>> functions_;
	};

	vision_service::vision_service() : impl_{ new impl }
	{
	}

	vision_service::~vision_service()
	{
		if (impl_)
		{
			delete impl_;
			impl_ = nullptr;
		}
	}

	param_string vision_service::name() const
	{
		return impl_->name();
	}

	param_string vision_service::version() const
	{
		return impl_->version();
	}

	param_vector<param_string> vision_service::get_available_functions() const
	{
		return impl_->get_available_functions();
	}

	unknown_object vision_service::execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const
	{
		return impl_->execute(function_name, params);
	}
}

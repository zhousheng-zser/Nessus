#include "vision_service_impl.hpp"

#include <mutex>
#include <functional>
#include <unordered_map>

#include <longinus/retina_net.hpp>
#include <irisviel/face_service.hpp>
#include <romancia/face_alignment.hpp>
#include <gaius/feature_extractor.hpp>
#include <cassius/feature_extractor.hpp>
#include <damocles/anti_spoofing.hpp>
#include <selene/feature_extractor.hpp>
#include <gungnir/yolo_net.hpp>
#include <leavepost/yolo_net.hpp>
#include <mjollner/ocr_net.hpp>
#include <valklyrs/yolov5s_net.hpp>
#include <heimdall/material_code.hpp>
#include <banshee/kcf_tracker.hpp>
#include <ring/material_code.hpp>
#include <plate/ocr_code.hpp>
#include <rail/classify_code.hpp>
#include <refvest/classify_code.hpp>
#include <flame/detect_code.hpp>
#include <sleep/detect_code.hpp>
#include <smoke/detect_code.hpp>
#include <onphone/detect_code.hpp>
#include <trespass/detect_code.hpp>
#include <helmet/detect_code.hpp>
#include <eledash/classify_code.hpp>
#include <ebike/detect_code.hpp>
#include <callsmoke/detect_code.hpp>
#include <genocr/txt_code.hpp>
#include <startorus/detect_code.hpp>
#include <valve/detect_code.hpp>
#include <needledash/ocr_code.hpp>
#include <phone/detect_code.hpp>
#include <workcloth/classify_code.hpp>

#include <iostream>

using namespace glasssix::gaius;
using namespace glasssix::cassius;
using namespace glasssix::romancia;
using namespace glasssix::irisviel;
using namespace glasssix::longinus;
using namespace glasssix::damocles;
using namespace glasssix::selene;
using namespace glasssix::gungnir;
using namespace glasssix::leavepost;
using namespace glasssix::mjollner;
using namespace glasssix::valklyrs;
using namespace glasssix::banshee;
using namespace glasssix::plate;
using namespace glasssix::rail;
using namespace glasssix::flame;
using namespace glasssix::sleep;
using namespace glasssix::smoke;
using namespace glasssix::onphone;
using namespace glasssix::trespass;
using namespace glasssix::helmet;
using namespace glasssix::eledash;
using namespace glasssix::ebike;
using namespace glasssix::callsmoke;
using namespace glasssix::needledash;
using namespace glasssix::phone;

namespace glasssix::exposing::nessus
{
	namespace
	{
		struct package_names
		{
			static constexpr utf8_string_view gaius{ u8"gaius" };
			static constexpr utf8_string_view cassius{ u8"cassius" };
			static constexpr utf8_string_view longinus{ u8"longinus" };
			static constexpr utf8_string_view romancia{ u8"romancia" };
			static constexpr utf8_string_view irisviel{ u8"irisviel" };
			static constexpr utf8_string_view damocles{ u8"damocles" };
			static constexpr utf8_string_view selene{ u8"selene" };
			static constexpr utf8_string_view gungnir{ u8"gungnir" };
			static constexpr utf8_string_view leavepost{ u8"leavepost" };
			static constexpr utf8_string_view mjollner{ u8"mjollner" };
			static constexpr utf8_string_view valklyrs{ u8"valklyrs" };
			static constexpr utf8_string_view heimdall{ u8"heimdall" };
			static constexpr utf8_string_view banshee{ u8"banshee" };
			static constexpr utf8_string_view ring{ u8"ring" };
			static constexpr utf8_string_view plate{ u8"plate" };
			static constexpr utf8_string_view rail{ u8"rail" };
			static constexpr utf8_string_view refvest{ u8"refvest" };
			static constexpr utf8_string_view flame{ u8"flame" };
			static constexpr utf8_string_view sleep{ u8"sleep" };
			static constexpr utf8_string_view smoke{ u8"smoke" };
			static constexpr utf8_string_view onphone{ u8"onphone" };
			static constexpr utf8_string_view trespass{ u8"trespass" };
			static constexpr utf8_string_view helmet{ u8"helmet" };
			static constexpr utf8_string_view eledash{ u8"eledash" };
			static constexpr utf8_string_view ebike{ u8"ebike" };
			static constexpr utf8_string_view callsmoke{ u8"callsmoke" };
			static constexpr utf8_string_view genocr{ u8"genocr" };
			static constexpr utf8_string_view startorus{ u8"startorus" };
			static constexpr utf8_string_view valve{ u8"valve" };
			static constexpr utf8_string_view needledash{ u8"needledash" };
			static constexpr utf8_string_view phone{ u8"phone" };
			static constexpr utf8_string_view workcloth{ u8"workcloth" };

		};

		struct function_names final
		{
			static constexpr utf8_string_view workcloth_new{ u8"workcloth.new" };
			static constexpr utf8_string_view workcloth_detect{ u8"workcloth.detect" };
			static constexpr utf8_string_view workcloth_delete{ u8"workcloth.delete" };
			static constexpr utf8_string_view phone_new{ u8"phone.new" };
			static constexpr utf8_string_view phone_detect{ u8"phone.detect" };
			static constexpr utf8_string_view phone_delete{ u8"phone.delete" };
			static constexpr utf8_string_view needledash_new{ u8"needledash.new" };
			static constexpr utf8_string_view needledash_detect{ u8"needledash.detect" };
			static constexpr utf8_string_view needledash_delete{ u8"needledash.delete" };
			static constexpr utf8_string_view valve_new{ u8"valve.new" };
			static constexpr utf8_string_view valve_detect{ u8"valve.detect" };
			static constexpr utf8_string_view valve_delete{ u8"valve.delete" };
			static constexpr utf8_string_view startorus_new{ u8"startorus.new" };
			static constexpr utf8_string_view startorus_detect{ u8"startorus.detect" };
			static constexpr utf8_string_view startorus_delete{ u8"startorus.delete" };
			static constexpr utf8_string_view genocr_new{ u8"genocr.new" };
			static constexpr utf8_string_view genocr_detect{ u8"genocr.detect" };
			static constexpr utf8_string_view genocr_delete{ u8"genocr.delete" };
			static constexpr utf8_string_view callsmoke_new{ u8"callsmoke.new" };
			static constexpr utf8_string_view callsmoke_detect{ u8"callsmoke.detect" };
			static constexpr utf8_string_view callsmoke_delete{ u8"callsmoke.delete" };
			static constexpr utf8_string_view ebike_new{ u8"ebike.new" };
			static constexpr utf8_string_view onphone_new{ u8"onphone.new" };
			static constexpr utf8_string_view gungnir_new{ u8"gungnir.new" };
			static constexpr utf8_string_view ebike_detect{ u8"ebike.detect" };
			static constexpr utf8_string_view ebike_delete{ u8"ebike.delete" };
			static constexpr utf8_string_view helmet_new{ u8"helmet.new" };
			static constexpr utf8_string_view helmet_detect{ u8"helmet.detect" };
			static constexpr utf8_string_view helmet_delete{ u8"helmet.delete" };
			static constexpr utf8_string_view eledash_new{ u8"eledash.new" };
			static constexpr utf8_string_view eledash_detect{ u8"eledash.detect" };
			static constexpr utf8_string_view eledash_delete{ u8"eledash.delete" };
			static constexpr utf8_string_view trespass_new{ u8"trespass.new" };
			static constexpr utf8_string_view trespass_detect{ u8"trespass.detect" };
			static constexpr utf8_string_view trespass_delete{ u8"trespass.delete" };
			static constexpr utf8_string_view flame_new{ u8"flame.new" };
			static constexpr utf8_string_view flame_detect{ u8"flame.detect" };
			static constexpr utf8_string_view flame_delete{ u8"flame.delete" };
			static constexpr utf8_string_view sleep_new{ u8"sleep.new" };
			static constexpr utf8_string_view sleep_detect{ u8"sleep.detect" };
			static constexpr utf8_string_view sleep_delete{ u8"sleep.delete" };
			static constexpr utf8_string_view smoke_new{ u8"smoke.new" };
			static constexpr utf8_string_view smoke_detect{ u8"smoke.detect" };
			static constexpr utf8_string_view smoke_delete{ u8"smoke.delete" };
	
			static constexpr utf8_string_view onphone_detect{ u8"onphone.detect" };
			static constexpr utf8_string_view onphone_delete{ u8"onphone.delete" };
			static constexpr utf8_string_view gungnir_delete{ u8"gungnir.delete" };
			static constexpr utf8_string_view gungnir_detect{ u8"gungnir.detect" };
			static constexpr utf8_string_view refvest_new{ u8"refvest.new" };
			static constexpr utf8_string_view refvest_detect{ u8"refvest.detect" };
			static constexpr utf8_string_view refvest_delete{ u8"refvest.delete" };

			static constexpr utf8_string_view rail_new{ u8"rail.new" };
			static constexpr utf8_string_view rail_detect{ u8"rail.detect" };
			static constexpr utf8_string_view rail_delete{ u8"rail.delete" };
			static constexpr utf8_string_view plate_new{ u8"plate.new" };
			static constexpr utf8_string_view plate_detect{ u8"plate.detect" };
			static constexpr utf8_string_view plate_trace_init{ u8"plate.trace_init" };
			static constexpr utf8_string_view plate_trace_update{ u8"plate.trace_update" };
			static constexpr utf8_string_view plate_recognize{ u8"plate.recognize" };
			static constexpr utf8_string_view plate_delete{ u8"plate.delete" };
			static constexpr utf8_string_view ring_new{ u8"ring.new" };
			static constexpr utf8_string_view ring_detect{ u8"ring.detect" };
			static constexpr utf8_string_view ring_delete{ u8"ring.delete" };
			static constexpr utf8_string_view gaius_new{ u8"gaius.new" };
			static constexpr utf8_string_view cassius_new{ u8"cassius.new" };
			static constexpr utf8_string_view longinus_new{ u8"longinus.new" };
			static constexpr utf8_string_view damocles_new{ u8"damocles.new" };
			static constexpr utf8_string_view romancia_new{ u8"romancia.new" };
			static constexpr utf8_string_view irisviel_new{ u8"irisviel.new" };
			static constexpr utf8_string_view selene_new{ u8"selene.new" };
			static constexpr utf8_string_view leavepost_new{ u8"leavepost.new" };
			static constexpr utf8_string_view mjollner_new{ u8"mjollner.new" };
			static constexpr utf8_string_view valklyrs_new{ u8"valklyrs.new" };
			static constexpr utf8_string_view heimdall_new{ u8"heimdall.new" };
			static constexpr utf8_string_view banshee_new{ u8"banshee.new" };
			static constexpr utf8_string_view gaius_delete{ u8"gaius.delete" };
			static constexpr utf8_string_view cassius_delete{ u8"cassius.delete" };
			static constexpr utf8_string_view longinus_delete{ u8"longinus.delete" };
			static constexpr utf8_string_view damocles_delete{ u8"damocles.delete" };
			static constexpr utf8_string_view romancia_delete{ u8"romancia.delete" };
			static constexpr utf8_string_view irisviel_delete{ u8"irisviel.delete" };
			static constexpr utf8_string_view selene_delete{ u8"selene.delete" };
			static constexpr utf8_string_view leavepost_delete{ u8"leavepost.delete" };
			static constexpr utf8_string_view mjollner_delete{ u8"mjollner.delete" };
			static constexpr utf8_string_view valklyrs_delete{ u8"valklyrs.delete" };
			static constexpr utf8_string_view heimdall_delete{ u8"heimdall.delete" };
			static constexpr utf8_string_view banshee_delete{ u8"banshee.delete" };
			static constexpr utf8_string_view gaius_forward{ u8"gaius.forward" };
			static constexpr utf8_string_view cassius_forward{ u8"cassius.forward" };
			static constexpr utf8_string_view selene_forward{ u8"selene.forward" };
			static constexpr utf8_string_view selene_get_model_type{ u8"selene.get_model_type" };
			static constexpr utf8_string_view longinus_detect{ u8"longinus.detect" };
			static constexpr utf8_string_view longinus_trace{ u8"longinus.trace" };
			static constexpr utf8_string_view longinus_center_scale_align_face{ u8"longinus.center_scale_alignFace" };
			static constexpr utf8_string_view damocles_spoofing_detect{ u8"damocles.spoofing_detect" };
			static constexpr utf8_string_view damocles_presentation_attack_detect{ u8"damocles.presentation_attack_detect" };
		
			static constexpr utf8_string_view romancia_align_face_128{ u8"romancia.alignFace128" };
			static constexpr utf8_string_view romancia_align_face{ u8"romancia.alignFace" };
			static constexpr utf8_string_view romancia_blur_detect{ u8"romancia.blur_detect" };
			static constexpr utf8_string_view romancia_mask_detect{ u8"romancia.mask_detect" };
			static constexpr utf8_string_view romancia_rotate{ u8"romancia.rotate" };
			static constexpr utf8_string_view irisviel_clear{ u8"irisviel.clear" };
			static constexpr utf8_string_view irisviel_remove_all{ u8"irisviel.remove_all" };
			static constexpr utf8_string_view irisviel_load_databases{ u8"irisviel.load_databases" };
			static constexpr utf8_string_view irisviel_record_count{ u8"irisviel.record_count" };
			static constexpr utf8_string_view irisviel_contains_key{ u8"irisviel.contains_key" };
			static constexpr utf8_string_view irisviel_try_get_record{ u8"irisviel.try_get_record" };
			static constexpr utf8_string_view irisviel_add_record{ u8"irisviel.add_record" };
			static constexpr utf8_string_view irisviel_add_records{ u8"irisviel.add_records" };
			static constexpr utf8_string_view irisviel_update_record{ u8"irisviel.update_record" };
			static constexpr utf8_string_view irisviel_update_records{ u8"irisviel.update_records" };
			static constexpr utf8_string_view irisviel_remove_record{ u8"irisviel.remove_record" };
			static constexpr utf8_string_view irisviel_remove_records{ u8"irisviel.remove_records" };
			static constexpr utf8_string_view irisviel_search{ u8"irisviel.search" };
			static constexpr utf8_string_view irisviel_search_nf{ u8"irisviel.search_nf" };
			static constexpr utf8_string_view leavepost_detect{ u8"leavepost.detect" };
			static constexpr utf8_string_view mjollner_detect{ u8"mjollner.detect" };
			static constexpr utf8_string_view valklyrs_detect{ u8"valklyrs.detect" };
			static constexpr utf8_string_view heimdall_detect{ u8"heimdall.detect" };
			static constexpr utf8_string_view banshee_init{ u8"banshee.init" };
			static constexpr utf8_string_view banshee_update{ u8"banshee.update" };
		};
	}

	class vision_service_impl::impl
	{
	public:
		impl()
		{
			// New
			functions_.insert_or_assign(function_names::gungnir_new, std::bind(&impl::gungnir_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::phone_new, std::bind(&impl::needledash_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::needledash_new, std::bind(&impl::needledash_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::valve_new, std::bind(&impl::valve_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::startorus_new, std::bind(&impl::startorus_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::genocr_new, std::bind(&impl::genocr_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::callsmoke_new, std::bind(&impl::ebike_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::ebike_new, std::bind(&impl::ebike_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::helmet_new, std::bind(&impl::helmet_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::eledash_new, std::bind(&impl::eledash_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::trespass_new, std::bind(&impl::trespass_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::flame_new, std::bind(&impl::flame_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::sleep_new, std::bind(&impl::sleep_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::refvest_new, std::bind(&impl::refvest_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::rail_new, std::bind(&impl::rail_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::plate_new, std::bind(&impl::plate_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::ring_new, std::bind(&impl::ring_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::gaius_new, std::bind(&impl::gaius_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::cassius_new, std::bind(&impl::cassius_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::longinus_new, std::bind(&impl::longinus_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::damocles_new, std::bind(&impl::damocles_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_new, std::bind(&impl::romancia_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_new, std::bind(&impl::irisviel_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::selene_new, std::bind(&impl::selene_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::leavepost_new, std::bind(&impl::leavepost_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::mjollner_new, std::bind(&impl::mjollner_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::valklyrs_new, std::bind(&impl::valklyrs_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::heimdall_new, std::bind(&impl::heimdall_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::banshee_new, std::bind(&impl::banshee_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::smoke_new, std::bind(&impl::smoke_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::onphone_new, std::bind(&impl::onphone_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::workcloth_new, std::bind(&impl::workcloth_new, this, std::placeholders::_1));
			// Delete

			functions_.insert_or_assign(function_names::workcloth_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::gungnir_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::onphone_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::smoke_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::phone_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::needledash_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::valve_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::startorus_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::genocr_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::callsmoke_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::ebike_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::helmet_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::eledash_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::trespass_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::flame_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::sleep_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::refvest_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::rail_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::plate_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::ring_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::gaius_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::cassius_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::longinus_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::damocles_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::romancia_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::selene_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::leavepost_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::mjollner_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::valklyrs_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::heimdall_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::banshee_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));

			// Business
			functions_.insert_or_assign(function_names::workcloth_detect, std::bind(&impl::workcloth_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::gungnir_detect, std::bind(&impl::gungnir_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::smoke_detect, std::bind(&impl::smoke_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::onphone_detect, std::bind(&impl::onphone_detect, this, std::placeholders::_1));					
			functions_.insert_or_assign(function_names::phone_detect, std::bind(&impl::phone_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::needledash_detect, std::bind(&impl::needledash_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::valve_detect, std::bind(&impl::valve_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::startorus_detect, std::bind(&impl::startorus_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::genocr_detect, std::bind(&impl::genocr_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::callsmoke_detect, std::bind(&impl::ebike_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::ebike_detect, std::bind(&impl::ebike_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::helmet_detect, std::bind(&impl::helmet_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::trespass_detect, std::bind(&impl::trespass_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::sleep_detect, std::bind(&impl::sleep_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::eledash_detect, std::bind(&impl::eledash_detect, this, std::placeholders::_1));	
			functions_.insert_or_assign(function_names::flame_detect, std::bind(&impl::flame_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::refvest_detect, std::bind(&impl::refvest_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::rail_detect, std::bind(&impl::rail_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::plate_detect, std::bind(&impl::plate_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::plate_trace_init, std::bind(&impl::plate_trace_init, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::plate_trace_update, std::bind(&impl::plate_trace_update, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::plate_recognize, std::bind(&impl::plate_recognize, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::ring_detect, std::bind(&impl::ring_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::damocles_spoofing_detect, std::bind(&impl::damocles_spoofing_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::damocles_presentation_attack_detect, std::bind(&impl::damocles_presentation_attack_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::longinus_detect, std::bind(&impl::longinus_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::leavepost_detect, std::bind(&impl::leavepost_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::mjollner_detect, std::bind(&impl::mjollner_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::valklyrs_detect, std::bind(&impl::valklyrs_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::heimdall_detect, std::bind(&impl::heimdall_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::banshee_init, std::bind(&impl::banshee_init, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::banshee_update, std::bind(&impl::banshee_update, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::longinus_trace, std::bind(&impl::longinus_trace, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::longinus_center_scale_align_face, std::bind(&impl::longinus_center_scale_align_face, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_align_face_128, std::bind(&impl::romancia_align_face_128, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_align_face, std::bind(&impl::romancia_align_face, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_blur_detect, std::bind(&impl::romancia_blur_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_mask_detect, std::bind(&impl::romancia_mask_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_rotate, std::bind(&impl::romancia_rotate, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::gaius_forward, std::bind(&impl::gaius_extract_feature, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::cassius_forward, std::bind(&impl::cassius_extract_feature, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::selene_forward, std::bind(&impl::selene_extract_feature, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::selene_get_model_type, std::bind(&impl::selene_get_model_type, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_clear, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_clear, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_remove_all, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_remove_all, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_load_databases, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_load_databases, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_record_count, std::bind(&impl::irisviel_record_count, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_contains_key, std::bind(&impl::irisviel_contains_key, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_try_get_record, std::bind(&impl::irisviel_try_get_record, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_add_records, std::bind(&impl::irisviel_add_records, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_update_records, std::bind(&impl::irisviel_update_records, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_remove_records, std::bind(&impl::irisviel_remove_records, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_search, std::bind(&impl::irisviel_search, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_search_nf, std::bind(&impl::irisviel_search_nf, this, std::placeholders::_1));
		}

		~impl()
		{
			std::cout << "instances_.size: " << instances_.size() << std::endl;
		}

		param_string name() const
		{
			return u8"Glasssix Vision Service";
		}

		param_string version() const
		{
			return u8"1.0.0";
		}

		param_vector<param_string> available_functions() const
		{
			auto result = make_param_vector<param_string>();

			for (const auto& [key, value] : functions_)
			{
				result.push_back(key);
			}

			return result;
		}

		param_hash_map<guid, param_string> existing_instances() const
		{
			auto result = make_param_hash_map<guid, param_string>();

			for (const auto& [key, value] : instances_)
			{
				result.add_or_update(key, std::get<param_string>(value));
			}

			return result;
		}

		unknown_object execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const
		{
			auto iter = functions_.find(function_name);

			return iter != functions_.end() ? iter->second(params) : throw abi_key_not_found{ function_name };
		}

	private:

		unknown_object workcloth_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			return add_instance(package_names::workcloth, make_exported_interface<glasssix::workcloth::classify_code>(models_directory, device));
		}

		unknown_object phone_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			return add_instance(package_names::phone, make_exported_interface<glasssix::phone::detect_code>(models_directory, device));
		}

		unknown_object needledash_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::needledash, make_exported_interface<glasssix::needledash::ocr_code>(models_directory, device));
		}

		unknown_object valve_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto factory_type = unbox<std::int32_t>(params.get_value(u8"factory_type"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();
			return add_instance(package_names::valve, make_exported_interface<valve::detect_code>(models_directory, factory_type, device, params_map_abi));
		}

		unknown_object startorus_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto factory_type = unbox<std::int32_t>(params.get_value(u8"factory_type"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();
			return add_instance(package_names::startorus, make_exported_interface<startorus::detect_code>(models_directory, factory_type, device, params_map_abi));
		}

		unknown_object genocr_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto factory_type = unbox<std::int32_t>(params.get_value(u8"factory_type"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto chardic_directory = unbox<param_string>(params.get_value(u8"chardic_directory"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();
			return add_instance(package_names::genocr, make_exported_interface<genocr::txt_code>(models_directory, chardic_directory, factory_type, device, params_map_abi));
		}

		unknown_object callsmoke_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::callsmoke, make_exported_interface<callsmoke::detect_code>(models_directory, device));
		}

		unknown_object ebike_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::ebike, make_exported_interface<ebike::detect_code>(models_directory, device));
		}
		

		unknown_object helmet_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::helmet, make_exported_interface<helmet::detect_code>(models_directory, device));
		}

		unknown_object eledash_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::eledash, make_exported_interface<eledash::classify_code>(models_directory, device));
		}

		unknown_object gungnir_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::gungnir, make_exported_interface<gungnir::yolo_net>(models_directory, device));
		}

		unknown_object smoke_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::smoke, make_exported_interface<smoke::detect_code>(models_directory, device));
		}

		unknown_object onphone_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::onphone, make_exported_interface<onphone::detect_code>(models_directory, device));
		}

		unknown_object sleep_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::sleep, make_exported_interface<sleep::detect_code>(models_directory, device));
		}
	
		unknown_object trespass_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));

			return add_instance(package_names::trespass, make_exported_interface<trespass::detect_code>(models_directory, device));
		}

		unknown_object flame_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::flame, make_exported_interface<flame::detect_code>(models_directory, device));
		}

		unknown_object refvest_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));

			return add_instance(package_names::refvest, make_exported_interface<refvest::classify_code>(models_directory, device));
		}

		unknown_object rail_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::rail, make_exported_interface<rail::classify_code>(models_directory, device));
		}

		unknown_object plate_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::plate, make_exported_interface<plate::ocr_code>(models_directory, device));
		}

		unknown_object ring_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto factory_type = unbox<std::int32_t>(params.get_value(u8"factory_type"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::ring, make_exported_interface<ring::material_code>(models_directory, factory_type, device));
		}

		unknown_object cassius_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto model_type = unbox<std::int32_t>(params.get_value(u8"model_type"));
			auto use_int8 = unbox<std::int32_t>(params.get_value(u8"use_int8"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto racy_path = model_type ? models_directory + (use_int8 ? u8"/unicorn_res101_int8.racy" : u8"/unicorn_res101.racy") : models_directory + (use_int8 ? u8"/unicorn_int8.racy" : u8"/unicorn.racy");
			return add_instance(package_names::cassius, make_exported_interface<cassius::feature_extractor>(model_type, racy_path, device, use_int8 ? true : false));
		}

		unknown_object gaius_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto use_int8 = unbox<std::int32_t>(params.get_value(u8"use_int8"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::gaius, make_exported_interface<gaius::feature_extractor>(models_directory + (use_int8 ? u8"/mobile_unicorn_int8.racy" : u8"/mobile_unicorn.racy"),
				models_directory + (use_int8 ? u8"/mobile_unicorn_mask_int8.racy" : u8"/mobile_unicorn_mask.racy"), device, use_int8 ? true : false));
		}

		unknown_object selene_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto model_type = unbox<std::int32_t>(params.get_value(u8"model_type"));
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto use_int8 = unbox<std::int32_t>(params.get_value(u8"use_int8"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::selene, make_exported_interface<selene::feature_extractor>(models_directory, model_type, device, use_int8 ? true : false));
		}

		unknown_object longinus_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto nms = unbox<float>(params.get_value(u8"nms"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::longinus, make_exported_interface<retina_net>(models_directory + u8"/longinus.racy", models_directory + u8"/pfld_land71_simp.racy", nms, device));
		}


		unknown_object damocles_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto model_type = unbox<std::int32_t>(params.get_value(u8"model_type"));
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::damocles, make_exported_interface<damocles::anti_spoofing>(models_directory, model_type, device));
		}

		unknown_object romancia_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

            return add_instance(package_names::romancia, make_exported_interface<face_alignment>(models_directory + u8"/blur_detection_best.racy", device));
        }

		unknown_object irisviel_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto single_database_capacity = unbox<std::int32_t>(params.get_value(u8"single_database_capacity"));
			auto dimension = unbox<std::int32_t>(params.get_value(u8"dimension"));
			auto working_directory = unbox<param_string>(params.get_value(u8"working_directory"));

			return add_instance(package_names::irisviel, make_exported_interface<face_service>(irisviel::face_service_implemention::lsh_algorithm, single_database_capacity, dimension, working_directory));
		}


			unknown_object leavepost_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			
			return add_instance(package_names::leavepost, make_exported_interface<leavepost::yolo_net>(models_directory, device));
		}

		unknown_object mjollner_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::mjollner, make_exported_interface<ocr_net>(models_directory + u8"/det_db_resnet18.racy", models_directory + u8"/rec_crnn_resnet34.racy", models_directory + u8"/ppocr_keys_v1.txt", device));
		}

		unknown_object valklyrs_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::valklyrs, make_exported_interface<yolov5s_net>(models_directory + u8"/yolov5s.racy", models_directory + u8"/vehicle_attri.racy", models_directory + u8"/person_attri.racy", device));
		}

		unknown_object heimdall_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto factory_type = unbox<std::int32_t>(params.get_value(u8"factory_type"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();
			return add_instance(package_names::heimdall, make_exported_interface<heimdall::material_code>(models_directory, factory_type, device, params_map_abi));
		}

		unknown_object banshee_new(const param_hash_map<param_string, unknown_object>& params)
		{

			return add_instance(package_names::banshee, make_exported_interface<kcf_tracker>());
		}

		unknown_object workcloth_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<workcloth::classify_code>(params);

			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height, params_map_abi);
		}

		unknown_object phone_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<phone::detect_code>(params);

			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));


			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height, params_map_abi);
		}

		unknown_object needledash_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<needledash::ocr_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto type = unbox<std::int32_t>(params.get_value(u8"type"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, type, x, y, roi_width, roi_height, params_map_abi);
		}

		unknown_object valve_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<valve::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			auto rois_abi = params.get_value(u8"rois").as<param_vector<int>>();
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();
			return instance.detect(image, channels, height, width, order, rois_abi, params_map_abi);
		}

		unknown_object startorus_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<startorus::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			auto rois_abi = params.get_value(u8"rois").as<param_vector<int>>();

			return instance.detect(image, channels, height, width, order, rois_abi);
		}

		unknown_object genocr_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<genocr::txt_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto top_five = unbox<std::int32_t>(params.get_value(u8"top_five"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));
			return instance.detect(image, channels, height, width, top_five, order, x, y, roi_width, roi_height);
		}

		unknown_object callsmoke_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<callsmoke::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, params_map_abi);
		}

		unknown_object ebike_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<ebike::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, params_map_abi);
		}

		unknown_object helmet_detect(const param_hash_map<param_string, unknown_object>& params)
		{

			constexpr std::int32_t channels = 3;
			auto instance = get_instance<helmet::detect_code>(params);
			auto image  = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height, params_map_abi);
		}

		unknown_object smoke_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<smoke::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height, params_map_abi);
		}

		unknown_object onphone_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<onphone::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height, params_map_abi);
		}

		unknown_object eledash_detect(const param_hash_map<param_string, unknown_object>& params)
		{

			constexpr std::int32_t channels = 3;
			auto instance = get_instance<eledash::classify_code>(params);
			auto image  = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width);
		}
		
		unknown_object trespass_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<trespass::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto channels = unbox<std::int32_t>(params.get_value(u8"channels"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			return instance.detect(image, channels, height, width);
		}

		unknown_object flame_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<flame::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));


			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height, params_map_abi);
		}

		unknown_object sleep_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<sleep::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height, params_map_abi);
		}


		unknown_object refvest_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<refvest::classify_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto channels = unbox<std::int32_t>(params.get_value(u8"channels"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height);
		}
		     
		unknown_object gungnir_detect(const param_hash_map<param_string, unknown_object> &params)
        {
            constexpr std::int32_t channels = 3;
            auto instance = get_instance<gungnir::yolo_net>(params);
            auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
            auto height = unbox<std::int32_t>(params.get_value(u8"height"));
            auto width = unbox<std::int32_t>(params.get_value(u8"width"));
            auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.detect(image, channels, height, width, order);
		}




		unknown_object rail_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<rail::classify_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, params_map_abi);
		}

		unknown_object plate_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<plate::ocr_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, order, x, y, roi_width, roi_height, params_map_abi);

		}

		unknown_object plate_recognize(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<plate::ocr_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.recognize(image, channels, height, width, order);
		}

		unknown_object plate_trace_init(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<plate::ocr_code >(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_w = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_h = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			instance.trace_init(image, channels, height, width, order, roi_x, roi_y, roi_w, roi_h);

			return unknown_object();
		}

		unknown_object plate_trace_update(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<plate::ocr_code >(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.trace_update(image, channels, height, width, order);

		}

		unknown_object ring_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<ring::material_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto border_orient = unbox<std::int32_t>(params.get_value(u8"border_orient"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();
			return instance.detect(image, channels, height, width, border_orient, order, x, y, roi_width, roi_height, params_map_abi);
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
			auto has_mask = unbox<std::int32_t>(params.get_value(u8"has_mask"));

			return instance.get(aligned_faces, num, order, has_mask ? true : false);
		}

		unknown_object selene_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<selene::feature_extractor>(params);
			auto aligned_faces = unbox<param_span<std::uint8_t>>(params.get_value(u8"aligned_faces"));
			auto num = unbox<std::int32_t>(params.get_value(u8"num"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.get(aligned_faces, num, order);
		}

		unknown_object selene_get_model_type(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<selene::feature_extractor>(params);

			return box(instance.get_model_type());
		}

		unknown_object longinus_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<retina_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto min_size = unbox<std::int32_t>(params.get_value(u8"min_size"));
			auto threshold = unbox<float>(params.get_value(u8"threshold"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto do_attributing = unbox<bool>(params.get_value(u8"do_attributing"));

			return instance.detect(image, channels, height, width, min_size, threshold, order, do_attributing);
		}

		unknown_object damocles_spoofing_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<anti_spoofing>(params);
			auto faces = params.get_value(u8"faces").as<param_vector<face_info>>();
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.spoofing_detect(faces, image, channels, height, width, order);
		}

		unknown_object damocles_presentation_attack_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<anti_spoofing>(params);
			auto action_cmd = unbox<std::int32_t>(params.get_value(u8"action_cmd"));
			auto face = params.get_value(u8"face").as<face_info>();
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return box(instance.presentation_attack_detect(action_cmd, face, image, channels, height, width, order));
		}

		unknown_object longinus_trace(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<retina_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto face = params.get_value(u8"face").as<face_info>();
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto result = instance.single_trace(face, image, channels, height, width, order);

			return result;
		}

		unknown_object longinus_center_scale_align_face(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<retina_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto scale = unbox<float>(params.get_value(u8"scale"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.center_scale_align(image, channels, height, width, scale, order);
		}

		unknown_object romancia_align_face_128(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<face_alignment>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto faces = params.get_value(u8"faces").as<param_vector<face_info>>();
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.align128(image, channels, height, width, faces, order);
		}

		unknown_object romancia_align_face(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<face_alignment>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto faces = params.get_value(u8"faces").as<param_vector<face_info>>();
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.align(image, channels, height, width, faces, order);
		}

		unknown_object romancia_blur_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<face_alignment>(params);
			auto faces = params.get_value(u8"faces").as<param_vector<face_info>>();
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.blur_detect(faces, image, channels, height, width, order);
		}

		unknown_object romancia_mask_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<face_alignment>(params);
			auto faces = params.get_value(u8"faces").as<param_vector<face_info>>();
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.mask_detect(faces, image, channels, height, width, order);
		}

        unknown_object romancia_rotate(const param_hash_map<param_string, unknown_object>& params)
        {
            constexpr std::int32_t channels = 3;
            auto instance = get_instance<face_alignment>(params);
            auto angle = unbox<float>(params.get_value(u8"angle"));
            auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
            auto height = unbox<std::int32_t>(params.get_value(u8"height"));
            auto width = unbox<std::int32_t>(params.get_value(u8"width"));
            auto order = unbox<std::int32_t>(params.get_value(u8"order"));

            return instance.rotate(angle, image, channels, height, width, order);
        }

      
	
        unknown_object leavepost_detect(const param_hash_map<param_string, unknown_object> &params)
        {
            auto instance = get_instance<leavepost::yolo_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto channels = unbox<std::int32_t>(params.get_value(u8"channels"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height);
		}


		unknown_object mjollner_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<ocr_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			return instance.detect(image, channels, height, width, order, x, y, roi_width, roi_height);
		}

		unknown_object valklyrs_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<yolov5s_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.detect(image, channels, height, width, order);
		}

		unknown_object heimdall_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<heimdall::material_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto top_five = unbox<std::int32_t>(params.get_value(u8"top_five"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			return instance.detect(image, channels, height, width, top_five, order, x, y, roi_width, roi_height);
		}

		unknown_object banshee_init(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<kcf_tracker>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			instance.init_trace(image, width, height, x, y, roi_width, roi_height);
			return unknown_object();
		}

		unknown_object banshee_update(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<kcf_tracker>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			return instance.update(image, width, height);
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

		unknown_object irisviel_record_count(const param_hash_map<param_string, unknown_object>& params)
		{
			return box(get_instance<face_service>(params).record_count());
		}

		unknown_object irisviel_contains_key(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_service>(params);
			auto key = unbox<param_string>(params.get_value(u8"key"));

			return box(get_instance<face_service>(params).contains_key(key));
		}

		unknown_object irisviel_try_get_record(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_service>(params);
			auto key = unbox<param_string>(params.get_value(u8"key"));

			return get_instance<face_service>(params).try_get_record(key);
		}

		exposing::param_vector<bool> irisviel_add_records(const param_hash_map<param_string, unknown_object>& params)
		{
			return irisviel_add_or_update_records_helper(params, false);
		}

		exposing::param_vector<bool> irisviel_update_records(const param_hash_map<param_string, unknown_object>& params)
		{
			return irisviel_add_or_update_records_helper(params, true);
		}

		exposing::param_vector<bool> irisviel_remove_records(const param_hash_map<param_string, unknown_object>& params)
		{
			auto keys = params.get_value(u8"keys").as<param_vector<param_string>>();

			return get_instance<face_service>(params).remove_records(keys);
		}

		unknown_object irisviel_search(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_service>(params);
			auto feature = params.get_value(u8"feature").as<param_vector<float>>();

			unknown_object assuming_top{ nullptr };
			unknown_object assuming_min_similarity{ nullptr };
			bool has_top = params.try_get_value(u8"top", assuming_top) && assuming_top;
			bool has_min_similarity = params.try_get_value(u8"min_similarity", assuming_min_similarity) && assuming_min_similarity;

			if (has_top && has_min_similarity)
			{
				return instance.search(feature, unbox<float>(assuming_min_similarity), unbox<std::uint32_t>(assuming_top));
			}

			if (has_top)
			{
				return instance.search(feature, unbox<std::uint32_t>(assuming_top));
			}

			if (has_min_similarity)
			{
				return instance.search(feature, unbox<float>(assuming_min_similarity));
			}

			throw abi_invalid_argument{ "Missing required parameters: top or min_similarity." };
		}

		unknown_object irisviel_search_nf(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_service>(params);
			auto feature = params.get_value(u8"feature").as<param_vector<float>>();

			unknown_object assuming_top{ nullptr };
			unknown_object assuming_min_similarity{ nullptr };
			bool has_top = params.try_get_value(u8"top", assuming_top) && assuming_top;
			bool has_min_similarity = params.try_get_value(u8"min_similarity", assuming_min_similarity) && assuming_min_similarity;

			if (has_top && has_min_similarity)
			{
				return instance.search_nf(feature, unbox<float>(assuming_min_similarity), unbox<std::uint32_t>(assuming_top));
			}

			throw abi_invalid_argument{ "Missing required parameters: top and min_similarity." };
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

		exposing::param_vector<bool> irisviel_add_or_update_records_helper(const param_hash_map<param_string, unknown_object>& params, bool update)
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
				return instance.update_records(records);
			}
			else
			{
				return instance.add_records(records);
			}
		}

		unknown_object add_instance(utf8_string_view package_name, const unknown_object& instance)
		{
			auto id = create_guid_from_bytes(meta::to_array(reinterpret_cast<std::size_t>(get_abi(instance))));
			{
				std::scoped_lock lock{ mutex_ };

				return (instances_.insert_or_assign(id, std::tuple{ package_name, instance }), box(id));
			}
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

		template <typename T>
		T get_instance(const guid& id)
		{
			std::scoped_lock lock{ mutex_ };
			auto iter = instances_.find(id);

			return iter != instances_.end() ? std::get<unknown_object>(iter->second).as<T>() : throw abi_key_not_found{ exposing::format(u8"Cannot find instance: {}.", to_param_string(id)) };
		}

		template <typename T>
		T get_instance(const param_hash_map<param_string, unknown_object>& params)
		{
			return get_instance<T>(unbox<guid>(params.get_value(u8"object_id")));
		}

		std::mutex mutex_;
		std::unordered_map<guid, std::tuple<param_string, unknown_object>> instances_;
		std::unordered_map<param_string, std::function<unknown_object(const param_hash_map<param_string, unknown_object>&)>> functions_;
	};

	vision_service_impl::vision_service_impl() : impl_{ std::make_unique<impl>() }
	{
	}

	vision_service_impl::~vision_service_impl()
	{
	}

	param_string vision_service_impl::name() const
	{
		return impl_->name();
	}

	param_string vision_service_impl::version() const
	{
		return impl_->version();
	}

	param_vector<param_string> vision_service_impl::available_functions() const
	{
		return impl_->available_functions();
	}

	param_hash_map<guid, param_string> vision_service_impl::existing_instances() const
	{
		return impl_->existing_instances();
	}

	unknown_object vision_service_impl::execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const
	{
		return impl_->execute(function_name, params);
	}
}

/*
* Copyright (c) 2011 Sveriges Television AB <info@casparcg.com>
*
* This file is part of CasparCG (www.casparcg.com).
*
* CasparCG is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CasparCG is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CasparCG. If not, see <http://www.gnu.org/licenses/>.
*
* Author: Robert Nagy, ronag89@gmail.com
*/

#include "../StdAfx.h"

#include "frame_consumer.h"

namespace caspar { namespace core {
		
std::vector<const consumer_factory_t> g_factories;

void register_consumer_factory(const consumer_factory_t& factory)
{
	g_factories.push_back(factory);
}

spl::shared_ptr<core::frame_consumer> create_consumer(const std::vector<std::wstring>& params)
{
	if(params.empty())
		BOOST_THROW_EXCEPTION(invalid_argument() << arg_name_info("params") << arg_value_info(""));
	
	auto consumer = frame_consumer::empty();
	std::any_of(g_factories.begin(), g_factories.end(), [&](const consumer_factory_t& factory) -> bool
		{
			try
			{
				consumer = factory(params);
			}
			catch(...)
			{
				CASPAR_LOG_CURRENT_EXCEPTION();
			}
			return consumer != frame_consumer::empty();
		});

	if(consumer == frame_consumer::empty())
		BOOST_THROW_EXCEPTION(file_not_found() << msg_info("No match found for supplied commands. Check syntax."));

	return consumer;
}

const spl::shared_ptr<frame_consumer>& frame_consumer::empty()
{
	struct empty_frame_consumer : public frame_consumer
	{
		virtual bool send(const spl::shared_ptr<const data_frame>&) override {return false;}
		virtual void initialize(const video_format_desc&, int) override{}
		virtual std::wstring print() const override {return L"empty";}
		virtual bool has_synchronization_clock() const override {return false;}
		virtual int buffer_depth() const override {return 0;};
		virtual int index() const{return -1;}
		virtual boost::property_tree::wptree info() const override
		{
			boost::property_tree::wptree info;
			info.add(L"type", L"empty-consumer");
			return info;
		}
	};
	static spl::shared_ptr<frame_consumer> consumer = spl::make_shared<empty_frame_consumer>();
	return consumer;
}

}}
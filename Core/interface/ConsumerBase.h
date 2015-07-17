/* Copyright (c) 2013 - All Rights Reserved
 *   Thomas Hauth  <Thomas.Hauth@cern.ch>
 *   Joram Berger  <Joram.Berger@cern.ch>
 *   Dominik Haitz <Dominik.Haitz@kit.edu>
 */

#pragma once

#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "FilterBase.h"
#include "ProductBase.h"
#include "FilterResult.h"
#include "EventBase.h"
#include "Artus/Configuration/interface/SettingsBase.h"

#include "Artus/Core/interface/Cpp11Support.h"
#include "Artus/Core/interface/ProcessNodeBase.h"


class ConsumerBaseAccess;


/*
 The base class to implement your own Consumer which run within an Pipeline.
 This class gets the FilterResult, the created products, the settings and the input event ifself
 and can create the output you desire.
 It is not allowed to write to the settings, filter, event and products.
 */

class ConsumerBaseUntemplated : public ProcessNodeBase {
public:

	// this will allow the pipeline class to call the protected
	// base implemenations of ProcessEvent / Init and friends
	friend ConsumerBaseAccess;

	virtual ~ConsumerBaseUntemplated() {}

	/*
	 * Must return a unique id of the Consumer.
	 */
	virtual std::string GetConsumerId() const = 0;

protected:
	// will be implemented by the ConsumerBase class
	virtual void baseProcess( SettingsBase const& setting ) = 0;
	virtual void baseProcessEvent( EventBase const& evt, ProductBase const& prod,
			SettingsBase const& setting,
			FilterResult & fres) = 0;
	virtual void baseProcessFilteredEvent( EventBase const& evt, ProductBase const& prod,
			SettingsBase const& setting) = 0;
	virtual void baseInit ( SettingsBase const& settings ) = 0;
	virtual void baseFinish ( SettingsBase const& settings ) = 0;
};

class ConsumerBaseAccess {
public:
	ConsumerBaseAccess ( ConsumerBaseUntemplated & cb ) : m_cb(cb) {

	}

	void Process( SettingsBase const& settings ){
		m_cb.baseProcess( settings );
	}

	void ProcessEvent( EventBase const& evt, ProductBase const& prod, SettingsBase const& settings, FilterResult fres){
		m_cb.baseProcessEvent( evt, prod, settings, fres);
	}

	void ProcessFilteredEvent( EventBase const& evt, ProductBase const& prod, SettingsBase const& settings ){
		m_cb.baseProcessFilteredEvent( evt, prod, settings );
	}

	void Init ( SettingsBase const& settings ){
		m_cb.baseInit( settings );
	}

	void Finish ( SettingsBase const& settings ) {
		m_cb.baseFinish( settings );
	}

private:
	ConsumerBaseUntemplated & m_cb;
};


template<class TTypes >
class ConsumerBase: public ConsumerBaseUntemplated  {
public:

	typedef typename TTypes::event_type event_type;
	typedef typename TTypes::product_type product_type;
	typedef typename TTypes::setting_type setting_type;

	virtual ~ConsumerBase() {
	}

	virtual void Init ( setting_type const& settings ) {
		LOG(DEBUG) << "Initialize consumer \"" << this->GetConsumerId() << "\".";
	}

	/* this method is only called for events which have passed the filter imposed on the
	 * pipeline
	 */
	virtual void ProcessFilteredEvent(event_type const& event,
			product_type const& product,
			setting_type const& settings) {
	}

	/*
	 *  this method is called for all events
	 *  Depending on the outcome of the pipeline ( a filter might have stopped the producers
	 *  prematurely), some local products might not have been filled
	 */

	//add setting_type const& globalSettings here !!
	virtual void ProcessEvent(event_type const& event,
			product_type const& product,
			setting_type const& settings,
            FilterResult & result) {
	}

	/*
	 *  this method is called for pipeline which do not operate on event input but on data
	 *  generated by preceeding pipelines
	 */
	virtual void Process(setting_type const& setting) {
	}

	/*
	 * Called after the last event. Overwrite this to store your histograms etc. to disk
	 */
	virtual void Finish(setting_type const& setting) = 0;

	/*
	 * Return a reference to the settings used for this consumer
	 */
/*	setting_type const& GetPipelineSettings() const {
		return this->m_pipeline->GetSettings();
	}*/

	virtual ProcessNodeType GetProcessNodeType () const final override
	{
		return ProcessNodeType::Consumer;
	}

protected:

	virtual void baseProcess(SettingsBase const& setting) override {

		auto const& specSetting = static_cast < setting_type const&> ( setting );

		Process( specSetting );
	}

	virtual void baseProcessEvent(EventBase const& evt, ProductBase const& prod,
	                              SettingsBase const& setting, FilterResult & fres)	override
	{

		auto const& specEvent = static_cast < event_type const&> ( evt );
		auto const& specProd = static_cast < product_type const&> ( prod );
		auto const& specSetting = static_cast < setting_type const&> ( setting );

		ProcessEvent( specEvent, specProd, specSetting, fres );
	}

	virtual void baseProcessFilteredEvent(EventBase const& evt,
			ProductBase const& prod,
			SettingsBase const& setting ) {

		auto const& specEvent = static_cast < event_type const&> ( evt );
		auto const& specProd = static_cast < product_type const&> ( prod );
		auto const& specSetting = static_cast < setting_type const&> ( setting );

		ProcessFilteredEvent(specEvent, specProd, specSetting);
	}

	void baseInit(SettingsBase const& settings) override {
		auto const& specSettings = static_cast < setting_type const&> ( settings );

		this->Init ( specSettings );
	}

	void baseFinish (SettingsBase const& settings) override {
		auto const& specSettings = static_cast < setting_type const&> ( settings );

		this->Finish ( specSettings );
	}
};

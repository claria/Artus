
#pragma once

#include "Artus/Consumer/interface/NtupleConsumerBase.h"

/*
 * Fills NTuples with valueExtractors defined as lambda functions
 * This removes the string operations from its base class
 */

template<class TTypes>
class LambdaNtupleConsumer: public NtupleConsumerBase<TTypes> {

public:

	typedef typename TTypes::event_type event_type;
	typedef typename TTypes::product_type product_type;
	typedef typename TTypes::setting_type setting_type;
	
	typedef typename std::function<float(event_type const&, product_type const&)> float_extractor_lambda;

	virtual std::string GetConsumerId()
	{
		return "lambda_ntuple";
	}

	LambdaNtupleConsumer(std::map<std::string, float_extractor_lambda> valueExtractorMap = std::map<std::string, float_extractor_lambda>()) :
			NtupleConsumerBase<TTypes>(),
			m_valueExtractorMap(valueExtractorMap)
	{
	}

	virtual ~LambdaNtupleConsumer() {
	}

	virtual void Init(Pipeline<TTypes> * pset) ARTUS_CPP11_OVERRIDE {
		NtupleConsumerBase<TTypes>::Init(pset);
		
		float defaultValue = -999.0;
		float_extractor_lambda defaultExtractor = [&defaultValue](event_type const&, product_type const&) { return defaultValue; };
		
		// construct extractors vector
		m_valueExtractors.clear();
		m_valueExtractors.resize(this->m_quantitiesVector.size());
		transform(this->m_quantitiesVector.begin(), this->m_quantitiesVector.end(), m_valueExtractors.begin(),
		          [&](std::string quantity) { return(m_valueExtractorMap.count(quantity) > 0 ? m_valueExtractorMap[quantity] : defaultExtractor); });

	}

	virtual void ProcessFilteredEvent(event_type const& event, product_type const& product ) ARTUS_CPP11_OVERRIDE
	{
		// do not call NtupleConsumerBase<TTypes>::ProcessFilteredEvent due to different filling logic
		ConsumerBase<TTypes>::ProcessFilteredEvent(event, product);
		
		// preallocated vector
		std::vector<float> array (this->m_quantitiesVector.size());
		
		size_t arrayIndex = 0;
		for(typename std::vector<float_extractor_lambda>::iterator valueExtractor = m_valueExtractors.begin();
		    valueExtractor != m_valueExtractors.end(); ++valueExtractor)
		{
			array[arrayIndex] = (*valueExtractor)(event, product);
			arrayIndex++;
		}

		// add the array to the ntuple
		this->m_ntuple->Fill(&array[0]);
	}


private:
	std::map<std::string, float_extractor_lambda> m_valueExtractorMap;
	std::vector<float_extractor_lambda> m_valueExtractors;

};



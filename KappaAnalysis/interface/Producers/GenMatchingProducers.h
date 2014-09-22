
#pragma once

#include "Kappa/DataFormats/interface/Kappa.h"

#include "Artus/Core/interface/ProducerBase.h"


/** Abstract Producer class for trigger matching valid objects
 *
 *	Needs to run after the valid object producers.
 */
template<class TTypes, class TValidObject>
class GenMatchingProducerBase: public ProducerBase<TTypes>
{

public:
	
	enum class TauDecayMode : int
	{
	        NONE = -1,
		E   = 0,
		M   = 1,
		T   = 2
	};

	typedef typename TTypes::event_type event_type;
	typedef typename TTypes::product_type product_type;
	typedef typename TTypes::setting_type setting_type;
	
	GenMatchingProducerBase(std::map<TValidObject*, const KGenParticle*> product_type::*genMatchedObjects, //changed to KGenParticle from KDataLV
	                            std::vector<TValidObject*> product_type::*validObjects,
	                            std::vector<TValidObject*> product_type::*invalidObjects,
	                            TauDecayMode tauDecayMode,
	                            float (setting_type::*GetDeltaRGenMatchingObjects)(void) const,
				    bool (setting_type::*GetInvalidateNonGenMatchingObjects)(void) const/* , */
/* 				    std::string (setting_type::*GetMatchingAlgorithmusObjects)(void) const */
	                            ) :
		m_genMatchedObjects(genMatchedObjects),
		m_validObjects(validObjects),
		m_invalidObjects(invalidObjects),
		tauDecayMode(tauDecayMode),
		GetDeltaRGenMatchingObjects(GetDeltaRGenMatchingObjects),
	        GetInvalidateNonGenMatchingObjects(GetInvalidateNonGenMatchingObjects)/* , */
/* 		GetMatchingAlgorithmusObjects(GetMatchingAlgorithmusObjects) */
	{
	}

	virtual void Init(setting_type const& settings) ARTUS_CPP11_OVERRIDE 
	{
		ProducerBase<TTypes>::Init(settings);
		LambdaNtupleConsumer<TTypes>::Quantities["ratioGenMatched"] = [](event_type const & event, product_type const & product)
		{
			return product.m_ratioGenMatched;
		};
	}

	virtual void Produce(event_type const& event, product_type& product,
	                     setting_type const& settings) const ARTUS_CPP11_OVERRIDE
	{
	  float ratioGenMatched = 0;
	  // matching jets to partonen and getting the jet flavour
	  if(tauDecayMode == TauDecayMode::NONE)
	  {
	    if ((settings.*GetDeltaRGenMatchingObjects)() > 0.0)
		{
			// loop over all valid objects (jets) to check
			for (typename std::vector<TValidObject*>::iterator validObject = (product.*m_validObjects).begin();
			     validObject != (product.*m_validObjects).end();)
			{
				bool objectMatched = false;
				float deltaR = 0;
				// loop over all genParticles 
				assert(event.m_genParticles); //moved felix
				for (typename std::vector<KGenParticle>::const_iterator genParticle = event.m_genParticles->begin();
			             !objectMatched && genParticle != event.m_genParticles->end();++genParticle) 
				{
				  KGenParticles matching_algo_partons;
				  KGenParticles matching_phys_partons;
				  const KGenParticle * hardest_parton = NULL;
				  const KGenParticle * hardest_b_quark = NULL;
				  const KGenParticle * hardest_c_quark = NULL;
				  // only use genParticles with id 21, 1, -1, 2, -2, 3, -3, 4, -4, 5, -5
				  if ( (abs(genParticle->pdgId())==1 || abs(genParticle->pdgId())==2 || abs(genParticle->pdgId())==3 
					   || abs(genParticle->pdgId())==4 || abs(genParticle->pdgId())==5 
					   || abs(genParticle->pdgId())==21) )
				    {
				    // Algorithmic:
				    if (genParticle->status() != 3)
				      {
					LOG(INFO) << "algorithmic";
					deltaR = ROOT::Math::VectorUtil::DeltaR((*validObject)->p4, genParticle->p4);
					if (deltaR<(settings.*GetDeltaRGenMatchingObjects)())
					  {
					    matching_algo_partons.push_back(*genParticle);
					    if (std::abs(genParticle->pdgId()) == 5)
					      { 
						if (hardest_b_quark == NULL)
						  hardest_b_quark = &(*genParticle);
						else if (genParticle->p4.Pt() > hardest_b_quark->p4.Pt())
						  hardest_b_quark = &(*genParticle);
					      }
					    else if (std::abs(genParticle->pdgId()) == 4)
					      { 
						if (hardest_c_quark == NULL)
						  hardest_c_quark = &(*genParticle);
						else if (genParticle->p4.Pt() > hardest_c_quark->p4.Pt())
						  hardest_c_quark = &(*genParticle);
							      }
					    else if (hardest_parton == NULL)
					      hardest_parton = &(*genParticle);
					    else if (genParticle->p4.Pt() > hardest_parton->p4.Pt())
					      hardest_parton = &(*genParticle);
					  }
				      }
				  // Physics:
				    else
				      {
					LOG(INFO) << "physics";
					deltaR = ROOT::Math::VectorUtil::DeltaR((*validObject)->p4, genParticle->p4);
					if (deltaR<(settings.*GetDeltaRGenMatchingObjects)())
					  matching_phys_partons.push_back(*genParticle);
				      } 
				    }
				// ALGORITHMIC DEFINITION
				if (matching_algo_partons.size() == 1)      // exactly one match
				  {
				    //if((settings.*GetMatchingAlgorithmusObjects)()=="algorithmic")
				          (product.*m_genMatchedObjects)[*validObject] = &(matching_algo_partons[0]);
				    objectMatched = true;
				  }
				else if (hardest_b_quark && hardest_b_quark->p4.Pt() > 0.)
				  {
				    //if((settings.*GetMatchingAlgorithmusObjects)()=="algorithmic")
				          (product.*m_genMatchedObjects)[*validObject] = &(*hardest_b_quark);
				    objectMatched = true;
				  }
				else if (hardest_c_quark && hardest_c_quark->p4.Pt() > 0.)
				  {
				    //if((settings.*GetMatchingAlgorithmusObjects)()=="algorithmic")
				          (product.*m_genMatchedObjects)[*validObject] = &(*hardest_c_quark);
				    objectMatched = true;
				  }
				else if (matching_algo_partons.size() != 0)
				  {
				    //if((settings.*GetMatchingAlgorithmusObjects)()=="algorithmic")
				          (product.*m_genMatchedObjects)[*validObject] = &(*hardest_parton);
				    objectMatched = true;
				  }
				// PHYSICS DEFINITION
				// flavour is only well defined if exactly ONE matching parton!
				if (matching_phys_partons.size() == 1)
				  {
				    //if((settings.*GetMatchingAlgorithmusObjects)()=="physic")
				          (product.*m_genMatchedObjects)[*validObject] = &(matching_phys_partons[0]);
				    objectMatched = true;	
				    //calculate ratio only for Physics definition
				    ratioGenMatched += 1./(product.*m_validObjects).size(); 
				    LOG(INFO) << "ratio" << ratioGenMatched;
				  }
				}				
				// invalidate the object if the trigger has not matched
				if ((! objectMatched) && (settings.*GetInvalidateNonGenMatchingObjects)())
				  {
				    (product.*m_invalidObjects).push_back(*validObject);
				    validObject = (product.*m_validObjects).erase(validObject);
				  }
				else
				  {
				    ++validObject;
				  }
			}
			// preserve sorting of invalid objects
			if ((settings.*GetInvalidateNonGenMatchingObjects)())
			  {
			    std::sort((product.*m_invalidObjects).begin(), (product.*m_invalidObjects).end(),
				      [](TValidObject const* object1, TValidObject const* object2) -> bool
				      { return object1->p4.Pt() > object2->p4.Pt(); });
			  }
		}
	  }
	  // matching e,m,t to gen taus
	  else
	  {
		if ((settings.*GetDeltaRGenMatchingObjects)() > 0.0)
		{
			// loop over all valid objects to check
			for (typename std::vector<TValidObject*>::iterator validObject = (product.*m_validObjects).begin();
			     validObject != (product.*m_validObjects).end();)
			{
				bool objectMatched = false;
				float deltaR = 0;
				// loop over all genTaus
				assert(event.m_genTaus); 
				for (typename std::vector<KDataGenTau>::const_iterator genTau = event.m_genTaus->begin();
			     !objectMatched && genTau != event.m_genTaus->end();++genTau) 
				{
					// only use genTaus that will decay into comparable particles
					if (this->MatchDecayMode(*genTau,tauDecayMode))
					{
						deltaR = ROOT::Math::VectorUtil::DeltaR((*validObject)->p4, genTau->p4_vis);
						if(deltaR<(settings.*GetDeltaRGenMatchingObjects)())
						{
							(product.*m_genMatchedObjects)[*validObject] = &(*genTau);
							ratioGenMatched += 1./(product.*m_validObjects).size();
							objectMatched = true;
						}
					}
				}
				// invalidate the object if the trigger has not matched
				if ((! objectMatched) && (settings.*GetInvalidateNonGenMatchingObjects)())
				{
					(product.*m_invalidObjects).push_back(*validObject);
					validObject = (product.*m_validObjects).erase(validObject);
				}
				else
				{
					++validObject;
				}
			}
			// preserve sorting of invalid objects
			if ((settings.*GetInvalidateNonGenMatchingObjects)())
			{
				std::sort((product.*m_invalidObjects).begin(), (product.*m_invalidObjects).end(),
				          [](TValidObject const* object1, TValidObject const* object2) -> bool
				          { return object1->p4.Pt() > object2->p4.Pt(); });
			}
		}
	  }
	  product.m_ratioGenMatched = ratioGenMatched;
	}

	virtual bool MatchDecayMode(KDataGenTau const &genTau, TauDecayMode tauDecayMode) const
	{
		bool decayModeMatched = false;
		switch(tauDecayMode) 
  		{	
		        case TauDecayMode::NONE: LOG(ERROR) << "tauDecayMode == NONE"; //used for jets
			case TauDecayMode::E: if (genTau.isElectronicDecay()) decayModeMatched = true;
    			case TauDecayMode::M: if (genTau.isMuonicDecay()) decayModeMatched = true;
			case TauDecayMode::T: if (genTau.isHadronicDecay()) decayModeMatched = true;
		};
		return decayModeMatched;
	}
	
private:
	std::map<TValidObject*, const KGenParticle*> product_type::*m_genMatchedObjects; //changed to KGenParticle from KDataLV
	std::vector<TValidObject*> product_type::*m_validObjects;
	std::vector<TValidObject*> product_type::*m_invalidObjects;
	TauDecayMode tauDecayMode;
	float (setting_type::*GetDeltaRGenMatchingObjects)(void) const;
	bool (setting_type::*GetInvalidateNonGenMatchingObjects)(void) const;
	//std::string (setting_type::*GetMatchingAlgorithmusObjects)(void) const;
	
	std::map<size_t, std::vector<std::string> > m_objectTriggerFiltersByIndex;
	std::map<std::string, std::vector<std::string> > m_objectTriggerFiltersByHltName;

};


/** Producer for gen matched electrons
 *  Required config tags:
 *  - DeltaRGenMatchingElectrons (default provided)
 *  - InvalidateNonMatchingElectrons (default provided)
 */
template<class TTypes>
class ElectronGenMatchingProducer: public GenMatchingProducerBase<TTypes, KDataElectron>
{

public:

	typedef typename TTypes::event_type event_type;
	typedef typename TTypes::product_type product_type;
	typedef typename TTypes::setting_type setting_type;
	
	virtual std::string GetProducerId() const ARTUS_CPP11_OVERRIDE {
		return "ElectronGenMatchingProducer";
	}

	ElectronGenMatchingProducer() :
		GenMatchingProducerBase<TTypes, KDataElectron>(&product_type::m_genMatchedElectrons,
		                                                   &product_type::m_validElectrons,
		                                                   &product_type::m_invalidElectrons,
		                                                   ElectronGenMatchingProducer::TauDecayMode::E,
		                                                   &setting_type::GetDeltaRGenMatchingElectrons,
		                                                   &setting_type::GetInvalidateNonGenMatchingElectrons/* , */
/* 							           &setting_type::GetMatchingAlgorithmusJets */)
	{
	}

};


/** Producer for gen matched muons
 *  Required config tags:
 *  - DeltaRGenMatchingMuons (default provided)
 *  - InvalidateNonMatchingMuons (default provided)
 */
template<class TTypes>
class MuonGenMatchingProducer: public GenMatchingProducerBase<TTypes, KDataMuon>
{

public:

	typedef typename TTypes::event_type event_type;
	typedef typename TTypes::product_type product_type;
	typedef typename TTypes::setting_type setting_type;
	
	virtual std::string GetProducerId() const ARTUS_CPP11_OVERRIDE {
		return "MuonGenMatchingProducer";
	}
	
	MuonGenMatchingProducer() :
		GenMatchingProducerBase<TTypes, KDataMuon>(&product_type::m_genMatchedMuons,
		                                               &product_type::m_validMuons,
		                                               &product_type::m_invalidMuons,
		                                               MuonGenMatchingProducer::TauDecayMode::M,
		                                               &setting_type::GetDeltaRGenMatchingMuons,
		                                               &setting_type::GetInvalidateNonGenMatchingMuons/* , */
/* 							       &setting_type::GetMatchingAlgorithmusJets */)
	{
	}

};


/** Producer for gen matched taus
 *  Required config tags:
 *  - DeltaRGenMatchingTaus (default provided)
 *  - InvalidateNonMatchingTaus (default provided)
 */
template<class TTypes>
class TauGenMatchingProducer: public GenMatchingProducerBase<TTypes, KDataPFTau>
{

public:

	typedef typename TTypes::event_type event_type;
	typedef typename TTypes::product_type product_type;
	typedef typename TTypes::setting_type setting_type;
	
	virtual std::string GetProducerId() const ARTUS_CPP11_OVERRIDE {
		return "TauGenMatchingProducer";
	}
	
	TauGenMatchingProducer() :
		GenMatchingProducerBase<TTypes, KDataPFTau>(&product_type::m_genMatchedTaus,
		                                                &product_type::m_validTaus,
		                                                &product_type::m_invalidTaus,
		                                                TauGenMatchingProducer::TauDecayMode::T,
		                                                &setting_type::GetDeltaRGenMatchingTaus,
		                                                &setting_type::GetInvalidateNonGenMatchingTaus/* , */
/* 							        &setting_type::GetMatchingAlgorithmusJets */)
	{
	}

};


/** Producer for gen matched jets with Physics Definition
 *  Required config tags:
 *  - DeltaRGenMatchingJets (default provided)
 *  - InvalidateNonMatchingJets (default provided) 
 *  - MatchingAlgorithmusJets (default provided)
 */
template<class TTypes>
class JetGenMatchingProducer: public GenMatchingProducerBase<TTypes, KDataPFJet>
{

public:

	typedef typename TTypes::event_type event_type;
	typedef typename TTypes::product_type product_type;
	typedef typename TTypes::setting_type setting_type;
	
	virtual std::string GetProducerId() const ARTUS_CPP11_OVERRIDE {
		return "JetGenPhysicMatchingProducer";
	}
	
	JetGenMatchingProducer() :
		GenMatchingProducerBase<TTypes, KDataPFJet>(&product_type::m_genMatchedJets,
		                                                &product_type::m_validJets,
		                                                &product_type::m_invalidJets,
		                                                JetGenMatchingProducer::TauDecayMode::NONE,
		                                                &setting_type::GetDeltaRGenMatchingJets,
							        &setting_type::GetInvalidateNonGenMatchingJets/* , */
/* 							        &setting_type::GetMatchingAlgorithmusJets */)
	{
	}

};

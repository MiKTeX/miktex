#pragma once


#include "location_type.h"


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"



//DEFINE_RESPONCE_TYPE(Rsp_FindLinkLocationList, std::vector<LinkLocation>);
//DEFINE_RESPONCE_TYPE(Rsp_LocationList, std::vector<lsLocation>);


namespace LocationListEither{

	typedef  std::pair< boost::optional<std::vector<lsLocation>> , boost::optional<std::vector<LocationLink> > > Either;
	
};
extern  void Reflect(Reader& visitor, LocationListEither::Either& value);
//DEFINE_RESPONCE_TYPE(Rsp_LocationListEither, LocationListEither::Either);

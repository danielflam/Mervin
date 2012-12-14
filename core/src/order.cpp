#include "indicator.h"

namespace indicator
{

	int		Order::newOrderID()
	{
		DataStoreINI & config = session->getDataStore()->getIniStore("config");
		int ID = config.readInt("order manager","Last ID", 0) + 1;
		config.writeInt("order manager","Last ID", ID);
		return ID;
	}
}
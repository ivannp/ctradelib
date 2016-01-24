#include <string>

#include "gtest/gtest.h"
#include "Poco/Data/SQLite/Connector.h"

int main(int argc, char ** argv)
{
   ::testing::InitGoogleTest(&argc, argv);
   Poco::Data::SQLite::Connector::registerConnector();
   int rc = RUN_ALL_TESTS();
   Poco::Data::SQLite::Connector::unregisterConnector();
   return rc;
}
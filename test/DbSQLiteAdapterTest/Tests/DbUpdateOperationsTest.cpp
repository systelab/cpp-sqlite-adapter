#include "stdafx.h"
#include "Utilities.h"

#include "DbAdapterInterface/IConnection.h"
#include "DbSQLiteAdapter/Connection.h"

#include "TestUtilities/DbAdapter/Mocks/MockConnectionConfiguration.h"


namespace db { namespace unit_test {

	using namespace systelab::db;
	using namespace testing;
	using namespace utilities;

	static const std::string UPDATE_TABLE_NAME = "UPDATE_TABLE";
	static const int UPDATE_TABLE_NUM_RECORDS = 25;

	/**
	 * Tests if the update operations over a table performed using the SQLite DB adapter
	 * work properly.
	 */
	class DbUpdateOperationsTest: public testing::Test
	{
	public:
		void SetUp()
		{
			m_db = loadDatabase();
			dropTable(*m_db, UPDATE_TABLE_NAME);
			createTable(*m_db, UPDATE_TABLE_NAME, UPDATE_TABLE_NUM_RECORDS);
		}

		void TearDown()
		{
			dropTable(*m_db, UPDATE_TABLE_NAME);
		}

		std::unique_ptr<IDatabase> loadDatabase()
		{
			systelab::test_utility::MockConnectionConfiguration connectionConfiguration;
			EXPECT_CALL(connectionConfiguration, getParameter("filepath")).WillRepeatedly(Return("sqlite-test.db"));

			systelab::db::sqlite::Connection dbConnection;
			return dbConnection.loadDatabase(connectionConfiguration);
		}

		ITable& getUpdateTable() const
		{
			return m_db->getTable(UPDATE_TABLE_NAME);
		}

	private:
		std::unique_ptr<IDatabase> m_db;
	};


	TEST_F(DbUpdateOperationsTest, testUpdateSingleRecord)
	{
		// Prepare the new values for the single record
		ITable& table = getUpdateTable();
		std::unique_ptr<IPrimaryKeyValue> primaryKeyValue = table.createPrimaryKeyValue();
		primaryKeyValue->getFieldValue("ID").setIntValue(4);

		std::unique_ptr<ITableRecord> record = table.getRecordByPrimaryKey(*primaryKeyValue);
		record->getFieldValue("FIELD_INT_INDEX").setIntValue(1234);
		record->getFieldValue("FIELD_INT_NO_INDEX").setIntValue(4321);
		record->getFieldValue("FIELD_STR_INDEX").setStringValue("UPDATED_STR");
		record->getFieldValue("FIELD_STR_NO_INDEX").setStringValue("STR_UPDATED");
		record->getFieldValue("FIELD_REAL").setDoubleValue(6789.1234);
		record->getFieldValue("FIELD_BOOL").setBooleanValue(false);
		record->getFieldValue("FIELD_DATE").setDateTimeValue(boost::posix_time::ptime(boost::gregorian::date(2015,9,6)));

		// Update the single record
		RowsAffected nRows = table.updateRecord(*record);
		ASSERT_EQ(nRows, 1);

		// Check that the record has been updated successfully
		std::unique_ptr<ITableRecord> updatedRecord = table.getRecordByPrimaryKey(*primaryKeyValue);
		ASSERT_EQ(updatedRecord->getFieldValue("FIELD_INT_INDEX").getIntValue(),		1234);
		ASSERT_EQ(updatedRecord->getFieldValue("FIELD_INT_NO_INDEX").getIntValue(),		4321);
		ASSERT_EQ(updatedRecord->getFieldValue("FIELD_STR_INDEX").getStringValue(),		std::string("UPDATED_STR"));
		ASSERT_EQ(updatedRecord->getFieldValue("FIELD_STR_NO_INDEX").getStringValue(),	std::string("STR_UPDATED"));
		ASSERT_EQ(updatedRecord->getFieldValue("FIELD_REAL").getDoubleValue(),			6789.1234);
		ASSERT_EQ(updatedRecord->getFieldValue("FIELD_BOOL").getBooleanValue(),			false);
		ASSERT_EQ(updatedRecord->getFieldValue("FIELD_DATE").getDateTimeValue(),		boost::posix_time::ptime(boost::gregorian::date(2015,9,6)));
	}

	TEST_F(DbUpdateOperationsTest, testUpdateNonExistingRecordAffectsZeroRows)
	{
		ITable& table = getUpdateTable();
		std::unique_ptr<ITableRecord> nonExistingRecord = table.createRecord();
		nonExistingRecord->getFieldValue("ID").setIntValue(-1);
		nonExistingRecord->getFieldValue("FIELD_INT_INDEX").setIntValue(123);

		RowsAffected nRows = table.updateRecord(*nonExistingRecord);
		ASSERT_EQ(nRows, 0);
	}

	TEST_F(DbUpdateOperationsTest, testUpdateMultipleRecords)
	{
		// Prepare the new values and the condition value
		ITable& table = getUpdateTable();
		std::unique_ptr<ITableRecord> auxRecord = table.createRecord();
		auxRecord->getFieldValue("FIELD_INT_INDEX").setIntValue(0);
		auxRecord->getFieldValue("FIELD_INT_NO_INDEX").setIntValue(123);
		auxRecord->getFieldValue("FIELD_STR_INDEX").setStringValue("NEW_VALUE");
		auxRecord->getFieldValue("FIELD_STR_NO_INDEX").setStringValue("NEW_VALUE2");
		auxRecord->getFieldValue("FIELD_REAL").setDoubleValue(777.888);
		auxRecord->getFieldValue("FIELD_BOOL").setBooleanValue(false);
		auxRecord->getFieldValue("FIELD_DATE").setDateTimeValue(boost::posix_time::ptime(boost::gregorian::date(2012,4,5)));

		std::vector<IFieldValue*> newValues;
		newValues.push_back( &(auxRecord->getFieldValue("FIELD_INT_NO_INDEX")) );
		newValues.push_back( &(auxRecord->getFieldValue("FIELD_STR_INDEX")) );
		newValues.push_back( &(auxRecord->getFieldValue("FIELD_STR_NO_INDEX")) );
		newValues.push_back( &(auxRecord->getFieldValue("FIELD_REAL")) );
		newValues.push_back( &(auxRecord->getFieldValue("FIELD_BOOL")) );
		newValues.push_back( &(auxRecord->getFieldValue("FIELD_DATE")) );

		std::vector<IFieldValue*> conditionValues;
		conditionValues.push_back( &(auxRecord->getFieldValue("FIELD_INT_INDEX")) );

		// Update multiple records
		int expectedAffectedRows = (int) ceil (UPDATE_TABLE_NUM_RECORDS / 7.);
		RowsAffected nRows = table.updateRecordsByCondition(newValues, conditionValues);
		ASSERT_EQ(nRows, expectedAffectedRows);

		// Check records have been updated successfully
		std::unique_ptr<ITableRecordSet> updatedRecordset = table.filterRecordsByFields(conditionValues);
		while (updatedRecordset->isCurrentRecordValid())
		{
			const ITableRecord& updatedRecord = updatedRecordset->getCurrentRecord();
			ASSERT_EQ(updatedRecord.getFieldValue("FIELD_INT_NO_INDEX").getIntValue(),		123);
			ASSERT_EQ(updatedRecord.getFieldValue("FIELD_STR_INDEX").getStringValue(),		std::string("NEW_VALUE"));
			ASSERT_EQ(updatedRecord.getFieldValue("FIELD_STR_NO_INDEX").getStringValue(),	std::string("NEW_VALUE2"));
			ASSERT_EQ(updatedRecord.getFieldValue("FIELD_REAL").getDoubleValue(),			777.888);
			ASSERT_EQ(updatedRecord.getFieldValue("FIELD_BOOL").getBooleanValue(),			false);
			ASSERT_EQ(updatedRecord.getFieldValue("FIELD_DATE").getDateTimeValue(),			boost::posix_time::ptime(boost::gregorian::date(2012,4,5)));
			updatedRecordset->nextRecord();
		}
	}

}}

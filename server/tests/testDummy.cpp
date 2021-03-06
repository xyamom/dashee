#include "Servo/Dummy.h"
#include "ServoController/Dummy.h"
#include "dashee.h"

CPPUNIT_TEST_SUITE_REGISTRATION(dashee::test::ServoDummy);
CPPUNIT_TEST_SUITE_REGISTRATION(dashee::test::ServoControllerDummy);

int main(int argc, char ** argv)
{
    return dashee::test::run();
}

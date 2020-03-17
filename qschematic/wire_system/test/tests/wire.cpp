#include "doctest.h"
#include "wiremanager.h"
#include "wire.h"
#include "connector.h"
#include <QDebug>

TEST_SUITE("Wire")
{
    wire_system::wire_manager manager;

    TEST_CASE("Add and remove points")
    {
        // Create a wire
        auto wire = std::make_shared<wire_system::wire>();
        wire->append_point(QPointF(0, 10));
        wire->append_point(QPointF(23.2, 100));
        wire->append_point(QPointF(-123.4, 0.23));

        SUBCASE("Point can be added")
        {
            // Make sure the points are correct
            REQUIRE(wire->points_count() == 3);
            REQUIRE(wire->points().at(0) == QPointF(0, 10));
            REQUIRE(wire->points().at(1) == QPointF(23.2, 100));
            REQUIRE(wire->points().at(2) == QPointF(-123.4, 0.23));
        }

        SUBCASE("Points can be removed")
        {
            wire->remove_point(1);

            // Make sure the points has been removed
            REQUIRE(wire->points_count() == 2);
            REQUIRE(wire->points().at(0) == QPointF(0, 10));
            REQUIRE(wire->points().at(1) == QPointF(-123.4, 0.23));
        }
    }

    TEST_CASE("Wire can be simplified")
    {
        // Create a wire
        auto wire = std::make_shared<wire_system::wire>();
        wire->append_point(QPointF(10, 10));
        wire->append_point(QPointF(23.2, 10));
        wire->append_point(QPointF(1000, 10));
        wire->append_point(QPointF(1000, 10));

        REQUIRE(wire->points_count() == 4);

        wire->simplify();

        REQUIRE(wire->points_count() == 2);
    }

    TEST_CASE("Wires can be moved")
    {
        // Use a grid size of 1
        Settings settings = manager.settings();
        settings.gridSize = 1;
        manager.set_settings(settings);

        // Create a wire
        auto wire1 = std::make_shared<wire_system::wire>();
        wire1->append_point(QPointF(0, 10));
        wire1->append_point(QPointF(5, 10));
        wire1->append_point(QPointF(5, 5));
        wire1->append_point(QPointF(15, 5));
        manager.add_wire(wire1);

        // Create a second wire
        auto wire2 = std::make_shared<wire_system::wire>();
        wire2->append_point(QPointF(10, 5));
        wire2->append_point(QPointF(10, 10));
        wire2->append_point(QPointF(15, 10));
        manager.add_wire(wire2);

        // Generate the junctions
        manager.generate_junctions();

        // Move the first wire
        wire1->move(QVector2D(1, -1));

        // Make sure the first wire has moved correctly
        REQUIRE(wire1->points().at(0).toPointF() == QPointF(1, 9));
        REQUIRE(wire1->points().at(1).toPointF() == QPointF(6, 9));
        REQUIRE(wire1->points().at(2).toPointF() == QPointF(6, 4));
        REQUIRE(wire1->points().at(3).toPointF() == QPointF(16, 4));

        // Make sure the second wire stayed on the first one
        REQUIRE(wire2->points().at(0).toPointF() == QPointF(11, 4));
        REQUIRE(wire2->points().at(1).toPointF() == QPointF(11, 10));
        REQUIRE(wire2->points().at(2).toPointF() == QPointF(15, 10));

        // Move the second wire
        wire2->move(QVector2D(2, 1));

        // Make sure the first wire hasn't changed
        REQUIRE(wire1->points().at(0).toPointF() == QPointF(1, 9));
        REQUIRE(wire1->points().at(1).toPointF() == QPointF(6, 9));
        REQUIRE(wire1->points().at(2).toPointF() == QPointF(6, 4));
        REQUIRE(wire1->points().at(3).toPointF() == QPointF(16, 4));

        // Make sure the second wire has moved and is still on the first one
        REQUIRE(wire2->points().at(0).toPointF() == QPointF(11, 4));
        REQUIRE(wire2->points().at(1).toPointF() == QPointF(11, 11));
        REQUIRE(wire2->points().at(2).toPointF() == QPointF(17, 11));
    }
}
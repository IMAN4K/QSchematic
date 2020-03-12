# Top-Level Items

When adding an `Item` to the `Scene`, there are two possibilities. If the `Item`
is a "Top-Level" Item (it has no parent), then you should use
`Scene::addItem()`. If the `Item` is the child of another `Item` then you need
to use the superclass' implementation `QGraphicsItem::addItem()`.

> This is needed to determine which Items should be moved by the scene and which
> are moved by their parent

# Wires

## Origin

![image origin](illustrations/wire_origin.png)

When a the shape of the wire changes, the origin has to be updated so that
it's always in the top-left corner.

This is done in the `Wire::updatePosition()` method. It calculated by how
much the origin has to move and moves all the points in the opposite
direction before moving the origin. In order the prevent the wire from moving
the junction on the wire the `Wire::_internalMove` flag is set before the
call to `setPos()`.

`Wire::updatePosition()` is called in `Scene::mouseReleaseEvent()` when moving
items, in `Scene::mouseDoubleClickEvent()` when creating a new wire and in
`Wire::calculateBoundingRect()`

## Movement

### Prevent origin update

When moving, the shape of the wire can sometimes change because a point has
to be moved to stay on a connector. This has to be prevented because if the
origin is updated while the wire is moving the wire will get moved to it's
new position and that will move it's origin, which means that on the next
mouseMoveEvent it will be moved to the wrong position. To fix that, the
origin is only updated on mouse release. (See Origin)

### Offset

`Wire::_offset` stores an offset from the grid. This is necessary when a
wire's left-most point is attached to a rotated node. In that case, the
wire's position will not be perfectly on the grid. The offset stores the by
how much a wire has to be moved from the snapped position so that we can add
that to the position in `Wire::itemChange()`.

### Connectors

When a wire is moved, it has stay connected to the connectors. This is done
by moving the attached points in the opposite direction it is moving.

This is done it `Wire::itemChange()` when `change` is
`ItemPositionHasChanged`. There is one situation where this is skipped, and
that is when we are adjusting the origin of the wire, in that case the
`Wire::_internalMove` boolean will be set.

### Junctions

![two connected wires moving](illustrations/wires_moving.png)

When a wire is moved it has to carefu:lly move it's own junctions and the
junction of the other wires that are on it.

This is done it `Wire::itemChange()` when `change` is `ItemPositionChange`.
There is one situation where this is skipped, and that is when we are
adjusting the origin of the wire, in that case the `Wire::_internalMove`
boolean will be set.

First the wire moves every of its own junction in the opposite way it is
moving so that the junction will stay at the same position they were. Then it
goes through all the junctions that are attached to it and moves them in the
direction it is moving. After that the algorithms described in the section
**Junctions** are responsible for moving the point when making the last
adjustments like moving the points back on to their connectors.

## Wire Point

### Moving

There are two ways of moving a point a wire: `Wire::movePointBy()` and
`Wire::move_point_to()`. The first one moves a point relative to it's position
and the other moves it to a absolute position on the scene. This is not the
only difference though, the first one will take care of some additional stuff
like making sure that right angles are preserved if this is expected. The
second one, `Wire::move_point_to()`, only moves the point to a position without
worrying about anything else.

> Try to never use `QList::indexOf()` to get the index of the point because
there might be multiple points at the same position.


### Point move command

When a point is moved by the user a `CommandWirepointMove` command is created
on the undo stack. The command stores the location of the points before and
after the move.

## Junctions

When one of the wires extremities (first or last point) is moved onto another
wire it becomes a junction. The logic related to the junctions is mostly
implemented in the `wire_manager` class and this will be elaborated on in the
**Nets** chapter.

### Updating junctions

![junctions being moved by other wire](illustrations/junctions.png)

When moving a point or segment we have to move all the junctions that are on
a part of the wire that is changing.

In `Wire::movePointBy()` we move the junctions that are on the next and previous
segment but only if the segment is horizontal or vertical. If it's not, it
will be taken care of by `Wire::move_point_to()`.

In `Wire::moveLineSegmentBy()` we move the junctions that are on the segment.

In `Wire::move_point_to()` we move the junctions that are on the point we are
moving and those that are on the segments attached to the point. If the
junction has already moved by one of the previous algorithms this won't do
anything because the point will technically no longer be on the segment.

## Connectors

When the extremity of a wire is moved onto a connector it gets connected to it.
This is implemented in `wire_manager::attach_wire_to_connector()`. There are quite
a few places this has to be used.

The most straightforward being in `wire_manager::wirePointMovedByUser()`. As
the name suggests, this method gets called whenever a point is moved by the
user.

In order to correctly connect wires when a new wire is created or when a
`CommandItemRemove` is undone, the connections are also updated in
`WireNet::addWire()`.

To connect wires while creating wires, `Scene::mousePressEvent()` also checks
for connections.

When loading a file all the connections have to be generated from the
position of the wires and the connectors. This is done in
`wire_manager::generateConnections()`.

It is also possible to connect a connector to a point by moving it onto the
point. This is done in `Scene::mouseReleaseEvent()` by calling
`updateNodeConnections()`.

## Nets

### Moving a point

When a point is moved away or onto another wire they have to be connected or
disconnected accordingly. This is done in `wire_manager::wirePointMovedByUser()`.
If two wires have to be connected it calls `wire_manager::connect_wire()` and if
they have to be disconnected it calls `wire_manager::disconnectWire()`.

### Adding a wire

When `CommandItemAdd::redo()` is called it checks if a wire has a previous net
stored in `Wire::net()`. If there is already one, it makes sure that it's in
the scene and adds the wire to it. If it has no previous net, a new net has
to be created but this is delegated to the `wire_manager` by calling `addWire()`.

#### Undoing

When the command is undone, it simply calls `wire_manager::removeWire()`.

### Removing a wire

When a wire is removed by a `CommandItemRemove`, `wire_manager::removeWire()`
is called. This removes the wire from the scene, detaches it from every
connector, disconnects it from all the wires it is attached to
(`wire_manager::disconnectWire()`), removes it from its net and removes the net
from the scene if it was the last wire in it. The wire keeps a shared_ptr to
the net, so that if the command is undone, it can be added to the same net.

#### Undoing

When the wire is added back to the scene, we retrieve the net is was
originally in and if it's no longer it the scene we add it. Then the wire is
added to the net which will

### Connecting wires

When two wires from different nets are connected, we need to merge both nets.
It will always use the net of the wire the other one is connecting to. The
`wire_manager::connect_wire()` method will simply retrieve both nets and call
`wire_manager::mergeNets()` that will then move all the wired from one net to
the other.

### Disconnecting wires

The `wire_manager::disconnectWire(std::shared_ptr<Wire> w1,
std::shared_ptr<Wire> w2)` function disconnects a wire (w2) from another wire
(w1). It first calls `w1->disconnectWire(w2)` so that w1 no longer considers
w2 as being connected to it. Then, it calls
`wire_manager::wiresConnectedTo(w1)` to get a list of all the wires that are
connected to w1 or that w1 is connected to. If there are wires in the net
that are not in that list, a new net has to be created and those wires must
be moved to it. This makes sure that if the wire that is being disconnected
is still part of the same net, it will stay in it.

It's important that if w1 is connected to w2, it has stay connected so that
the wires can be restored correctly.

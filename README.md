# Seven47 Software -- PolyZones Plugin
PolyZones is an Unreal Engine 5 plugin that allows you to draw overlap zones as complex polygonal shapes. These zones are useful for cases where you would need to know where an actor or point is, such as a building interior where you want to force walking.

![PolyZoneStar](https://user-images.githubusercontent.com/3581910/199859090-9ba9e7b9-4eda-4219-9d57-2de6a22b71ca.png)

## Using the PolyZones
### Using the PolyZones in an Actor
To receive Enter/Exit events on an actor, you will first need to add the PolyZone Interface to your actor. PolyZones automatically track actors that implement this interface, and the "EnterPolyZone" and "ExitPolyZone" interface events will be called when an actor meets the PolyZone's criteria.

![Interface](https://user-images.githubusercontent.com/3581910/199860162-8b30bae5-ff91-4e17-b4e8-4d459bd584b9.png)

### Using a PolyZone child blueprint
PolyZones have blueprint events for Enter/Exit intended to be used in children classes. It is recommended to create a child blueprint for each area type. For example, if you want a PolyZone that you can use in all building interiors, you would create "PolyZone_Interior". You could then pass configuration options such as "Force Walking" to actors that Enter/Exit this zone.

![PolyZoneChild](https://user-images.githubusercontent.com/3581910/199860986-b9d8b344-c1aa-4fbd-a69a-e24a5a13353e.png)

### Using a PolyZone via function calls
There are some included functions to pull data from the PolyZones by means other than the standard Enter/Exit events. Avoid using these functions on tick if actor tracking is turned off in the PolyZone config.

![Functions](https://user-images.githubusercontent.com/3581910/199863877-57856003-26b3-4a78-ae99-8153cf09c75d.png)



## License
**Standard MIT License, please see the LICENSE file.**

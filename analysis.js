/**
 * F-Secure "homework"
 *
 * Peter Zhang
 * August 4, 2016
 */


var _ = require('lodash'),
    Promise = require('bluebird'),
    lineReader = require('line-reader');

//debug:
//TODO: delete later
//
function dio(obj, indent)
{
  var result = "";
  if (indent == null) indent = "";

  for (var property in obj)
  {
    var value = obj[property];
    if (typeof value == 'string')
      value = "'" + value + "'";
    else if (typeof value == 'object')
    {
      if (value instanceof Array)
      {
        // Just let JS convert the Array to a string!
        value = "[ " + value + " ]";
      }
      else
      {
        // Recursive dump
        // (replace "  " by "\t" or something else if you prefer)
        var od = dio(value, indent + "  ");
        // If you like { on the same line as the key
        //value = "{\n" + od + "\n" + indent + "}";
        // If you prefer { and } to be aligned
        value = "\n" + indent + "{\n" + od + "\n" + indent + "}";
      }
    }
    result += indent + "'" + property + "' : " + value + ",\n";
  }
  return result.replace(/,\n$/, "");
}


//end tmp


var analysis = {
    observed : {}, //observed event ids, rhs=count
    observedDevices : {}, //obs dev ids-> rhs=timestamp of first visit
    lastSeenDevices : {}, //obs dev ids-> rhs=timestamp of last visit
    eventTypeStat : {}, //event type statistics, populated on the fly
    launchedDevices : {}, //launch type statistics, populated on the fly
    firstLaunches : 0,
    duplicatedEvents : 0,
    timestamp_mismatched: 0,
    deviceTypeCount: {
        android : 0,
        ios: 0,
        nonMobile: 0,
        other: 0
    }, //android/iOS/other count
    timestamps: [],
    visitorOrigin: {
        unknown : 0
    } //visitor origin (country-city)
};

/**
 * we can pinpoint unique device by device->deviceid
 */
function hashJSON(obj) {
    if(obj && obj.sender_info && obj.sender_info.device
           && obj.sender_info.device.device_id
           && obj.time
           && obj.time.create_timestamp
           && obj.event_id) {
        return obj.sender_info.device.device_id
               + obj.time.create_timestamp
               + obj.event_id;
    } else {
        console.error("error at obj.");
        console.error(obj);
        return -1;
    }
}

/**
 * continuous session analysis
 */
function persistentSessionCount(obj) {
}

/**
 * get users with overall longest time of usage
 * as defined:
 * ‘activity time’ max('first event – latest event')
 */
function longestNActivityTime(statObj, N) {
}

/**
 * get duplicated events, with counts
 */
function getDuplicatedEvents(statObj) {
}

/**
 * get histogram of usage frequency within a day
 * within given time range
 *
 * 1 bin = 1 hour of day
 *
 * args: statObj: the stat object
 * startDateTime, endDateTime : timestamp in integer
 */

function getDailyUsageHistogramInRange(statObj, startDateTime, endDateTime) {
}

//TODO: sort by event interaction per user


/**
 * main pipeline
 */
var eachLine = Promise.promisify(lineReader.eachLine);

eachLine('obfuscated_data', function (json) {
    //parse json
    Promise.resolve(JSON.parse(json))
    .then(function (parsedObject) {
        //event duplication count
        if (parsedObject.event_id) {
            if(analysis.observed[parsedObject.event_id]) {
                analysis.observed[parsedObject.event_id] += 1;
                throw new Error('duplicate event id');
                return null;
            } else {
                analysis.observed[parsedObject.event_id] = 1;
                //add timestamp to list:
                analysis.timestamps.push(parsedObject.timestamp);
            }
        }
        return parsedObject;

    }).then(function (parsedObject) {
        //type accum:
        if (!analysis.eventTypeStat[parsedObject.type]) {
            analysis.eventTypeStat[parsedObject.type] = 1;
        } else {
            analysis.eventTypeStat[parsedObject.type] += 1;
        }
        return parsedObject;

    }).then(function (parsedObject) {
        //device first-seen-time record
        if(!analysis.observedDevices[parsedObject.device.device_id]
                && parsedObject.time){
            //first seen:
            analysis.observedDevices[parsedObject.device.device_id] =
                parsedObject.timestamp;
        }
        //log event if event is launch:
        if(!analysis.launchedDevices[parsedObject.device.device_id]
                && parsedObject.type === "launch") {
            analysis.launchedDevices[parsedObject.device.device_id] = 1;
        }
        //update last seen timestamp:
        analysis.lastSeenDevices[parsedObject.device.device_id] =
                parsedObject.timestamp;
        //potentially could be a problem if create time > timestamp
        if(parsedObject.timestamp < parsedObject.time.send_timestamp) {
            analysis.timestamp_mismatched += 1;
        }

        return parsedObject;
    }).then(function (parsedObject) {
        //OS counting PER UNIQUE EVENT (not per unique device):
        //same data can also be fetched from sender_info
        //HUOM! data also includes non-mobile visitors
        if(_.has(parsedObject, "device.operating_system.kind")) {
            var device_name = parsedObject.device.operating_system.kind.toLowerCase();
            if(device_name.match("android")){
                analysis.deviceTypeCount.android+=1;
            } else if(device_name.match("ios")){
                analysis.deviceTypeCount.ios+=1;
            } else if(device_name.match("windows")
                    || device_name.match("osx")){
                analysis.deviceTypeCount.nonMobile+=1;
            } else {
                analysis.deviceTypeCount.other+=1;
            }
        }
        return parsedObject;
    }).then(function (parsedObject) {
        //country counting:
        //HUOM! some country fields might not be populated
        if(_.has(parsedObject, "sender_info.geo.country")){
            if(!_.has(analysis, "visitorOrigin."
                        + parsedObject.sender_info.geo.country)) {
                analysis.visitorOrigin[parsedObject.sender_info.geo.country] = {
                    aggregate : 1
                };
                if(parsedObject.sender_info.geo.city) {
                    analysis.visitorOrigin
                        [parsedObject.sender_info.geo.country]
                            [parsedObject.sender_info.geo.city] = 1;
                }
            } else {
                //country already exist
                analysis.visitorOrigin[parsedObject.sender_info.geo.country].aggregate += 1;
                //populate city
                if (parsedObject.sender_info.geo.city) {
                    if(!_.has(analysis, "visitorOrigin."
                                + parsedObject.sender_info.geo.country
                                + "."
                                + parsedObject.sender_info.geo.city)) {
                        analysis.visitorOrigin
                            [parsedObject.sender_info.geo.country]
                                [parsedObject.sender_info.geo.city] = 1;
                    } else {
                        analysis.visitorOrigin
                            [parsedObject.sender_info.geo.country]
                                [parsedObject.sender_info.geo.city] += 1;
                    }
                }
            }

        } else {
            //direct to unknown:
            analysis.visitorOrigin.unknown += 1;
        }
        return parsedObject;
    }).then(function (parsedObject) {

        return parsedObject;
    }).catch(function(err) {
        //count duplicate events
        if (err.message === 'duplicate event id') {
            analysis.duplicatedEvents += 1;
        }
        else {
            throw err;
        }
    });

//data aggregation complete, now do post processing
}).then(function() {
    //sort timestamps
    analysis.timestamps.sort();
    analysis.eventCount = _.keys(analysis.observed).length;
    analysis.uniqueDeviceCount = _.keys(analysis.observedDevices).length;
    analysis.firstLaunches = _.keys(analysis.launchedDevices).length;
}).then(function () {
    //debug: display
    console.log(dio(_.omit(analysis,[
                    "timestamps",
                    "observed",
                    "observedDevices",
                    "visitorOrigin",
                    "launchedDevices",
                    "lastSeenDevices"
                    ]), "  "));
});



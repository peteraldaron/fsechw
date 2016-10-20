/**
 * data analysis pipeline
 *
 * Peter Zhang
 * August 4, 2016
 */

var _ = require('lodash'),
    Promise = require('bluebird'),
    lineReader = require('line-reader');

//debug: pretty printer
//source:
//https://stackoverflow.com/questions/130404/javascript-data-formatting-pretty-printer
//credit: gprathour, PhiLho
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


var dataContainerPrototype = {
    observed : {}, //observed event ids, rhs=count
    observedDevices : {}, //obs dev ids-> rhs=timestamp of first visit
    lastSeenDevices : {}, //obs dev ids-> rhs=timestamp of last visit
    eventTypeStat : {}, //event type statistics, populated on the fly
    launchedDevices : {}, //launch type statistics, populated on the fly
    firstLaunches : 0,
    duplicatedEvents : 0,
    timestamp_mismatched: 0,
    deviceTypeCount: {
        android : {
            count : 0
        },
        ios: {
            count : 0
        },
        nonMobile: {
            count : 0
        },
        other: {
            count : 0
        }
    }, //android/iOS/other count
    timestamps: [],
    visitorOrigin: {
        unknown : {
            aggregate: 0
        }
    } //visitor origin (country-city)
};

//analysis for different products:
var pros = {};

/**
 * get users with overall longest time of usage
 * as defined:
 * ‘activity time’ max('first event – latest event')
 */
function longestNActivityTimeDevices(statObj, N) {
    return _
        .chain(statObj.observedDevices)
        .keys()
        .map(function(devid) {
            //debug:
            if(isNaN(statObj.lastSeenDevices[devid]) || isNaN(statObj.observedDevices[devid])){
                throw new Error("NAN at "+devid);
            }
            return [devid, statObj.lastSeenDevices[devid] - statObj.observedDevices[devid]];
        })
        .sortBy(function(item) {
            return item[1];
        })
        .slice(_.size(statObj.observedDevices) - N)
        .value();
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
    //statobj.timestamps must be sorted
    //start index is inclusive
    var startIdx = _.sortedIndexBy(statObj.timestamps, startDateTime),
        endIdx = _.sortedIndexBy(statObj.timestamps, endDateTime);
    return _
        .chain(statObj.timestamps)
        .map(function (time) {
            //to hours of day:
            var dateObj = new Date(time);
            //note: this is local time
            return dateObj.getHours();
        })
        .slice(startIdx, endIdx)
        .countBy()
        .value();
}

/**
 * get top N countries of user origin by number of events
 */
function getTopNCountriesByNumEvents(statObj, N) {
    return _
        .chain(statObj.visitorOrigin)
        .keys()
        .map(function (maa) {
            return [maa, statObj.visitorOrigin[maa].aggregate];
        })
        .sortBy(function(entry) { return entry[1];})
        .slice(_.size(statObj.visitorOrigin) - N)
        .value();
}

/**
 * main pipeline
 */
var eachLine = Promise.promisify(lineReader.eachLine);

eachLine('obfuscated_data', function (json) {
    //parse json
    var analysis = null;
    Promise.resolve(JSON.parse(json))
    .then(function (parsedObject) {
        //pre processing: product switching
        if (parsedObject.source) {
            if (!_.has(pros, parsedObject.source)){
                pros[parsedObject.source] = _.cloneDeep(dataContainerPrototype);
            }
            analysis = pros[parsedObject.source];
            return parsedObject;

        } else {
            throw new Error("error: product name not found");
            return null;
        }
    }).then(function (parsedObject) {
        //event duplication count
        if (parsedObject.event_id) {
            if(analysis.observed[parsedObject.event_id]) {
                analysis.observed[parsedObject.event_id] += 1;
                throw new Error('duplicate event id');
                return null;
            } else {
                analysis.observed[parsedObject.event_id] = 1;
                //add timestamp to list:
                analysis.timestamps.push(parsedObject.timestamp || parsedObject.time.send_timestamp);
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
                parsedObject.timestamp || parsedObject.time.send_timestamp;
        }
        //log event if event is launch:
        if(!analysis.launchedDevices[parsedObject.device.device_id]
                && parsedObject.type === "launch") {
            analysis.launchedDevices[parsedObject.device.device_id] = 1;
        }
        //update last seen timestamp:
        analysis.lastSeenDevices[parsedObject.device.device_id] =
                parsedObject.timestamp || parsedObject.time.send_timestamp;
        //potentially could be a problem if create time > timestamp
        if(parsedObject.timestamp && parsedObject.timestamp < parsedObject.time.send_timestamp) {
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
                analysis.deviceTypeCount.android.count+=1;

                //populate types of specific device
                analysis.deviceTypeCount.android[parsedObject.type] =
                    analysis.deviceTypeCount.android[parsedObject.type] ?
                    analysis.deviceTypeCount.android[parsedObject.type]+1
                    :1;
            } else if(device_name.match("ios")){
                analysis.deviceTypeCount.ios.count+=1;
                analysis.deviceTypeCount.ios[parsedObject.type] =
                    analysis.deviceTypeCount.ios[parsedObject.type] ?
                    analysis.deviceTypeCount.ios[parsedObject.type]+1
                    :1;
            } else if(device_name.match("windows")
                    || device_name.match("osx")){
                analysis.deviceTypeCount.nonMobile.count+=1;
                analysis.deviceTypeCount.nonMobile[parsedObject.type] =
                    analysis.deviceTypeCount.nonMobile[parsedObject.type] ?
                    analysis.deviceTypeCount.nonMobile[parsedObject.type]+1
                    :1;
            } else {
                analysis.deviceTypeCount.other.count+=1;
                analysis.deviceTypeCount.other[parsedObject.type] =
                    analysis.deviceTypeCount.other[parsedObject.type] ?
                    analysis.deviceTypeCount.other[parsedObject.type]+1
                    :1;
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
            analysis.visitorOrigin.unknown.aggregate += 1;
        }
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
    _.forEach(pros, function(analysis) {
        //sort timestamps
        analysis.timestamps.sort();
        analysis.eventCount = _.keys(analysis.observed).length;
        analysis.uniqueDeviceCount = _.keys(analysis.observedDevices).length;
        analysis.firstLaunches = _.keys(analysis.launchedDevices).length;
    });
}).then(function () {
    //display data:
    _.forEach(_.keys(pros), function(name) {
        var analysis = pros[name];
        console.log(name);
        console.log(dio(_.omit(analysis,[
                        "timestamps",
                        "observed",
                        "observedDevices",
                        "visitorOrigin",
                        "launchedDevices",
                        "lastSeenDevices"
                        ]), "  "));
        //display top 5 longest activity users
        console.log(longestNActivityTimeDevices(analysis, 5));
        console.log(getDailyUsageHistogramInRange(analysis, 0, 1470316951330));
        console.log(getTopNCountriesByNumEvents(analysis, 10));
    });

});



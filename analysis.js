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
    observed : {}, //observed event ids
    duplicate_count : {}, //duplicated event ids with count
    observed_devices : {}, //obs dev ids-> rhs=timestamp of first visit
    last_seen_devices : {}, //obs dev ids-> rhs=timestamp of last visit
    event_type_stat : {}, //event type statistics, populated on the fly
    device_type_count: {
        android : 0,
        ios: 0,
        other: 0
    }, //android/iOS/other count
    visitor_origin: {
        unknown : 0
    } //visitor origin (country-city)
};

var inputFile = "tf";

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
 * as define:
 * ‘activity time’ max('first event – latest event')
 */
function longestNActivityTime(statObj, N) {
}


/**
 * main pipeline
 */
var eachLine = Promise.promisify(lineReader.eachLine);

eachLine('tf', function (json) {
    //parse json
    Promise.resolve(JSON.parse(json))
    .then(function (parsedObject) {
        //event duplication count
        if (parsedObject.event_id) {
            if(analysis.observed[parsedObject.event_id]) {
                if(!analysis.duplicate_count[parsedObject.event_id]) {
                    analysis.duplicate_count[parsedObject.event_id] = 1;
                } else {
                    analysis.duplicate_count[parsedObject.event_id] += 1;
                }
            } else {
                analysis.observed[parsedObject.event_id] = 1;
            }
        }
        return parsedObject;

    }).then(function (parsedObject) {
        //type accum:
        if (!analysis.event_type_stat[parsedObject.type]) {
            analysis.event_type_stat[parsedObject.type] = 1;
        } else {
            analysis.event_type_stat[parsedObject.type] += 1;
        }
        return parsedObject;

    }).then(function (parsedObject) {
        //device first-seen-time record
        if(!analysis.observed_devices[parsedObject.device_id]
                && parsedObject.time){
            //first seen:
            analysis.observed_devices[parsedObject.device_id] =
                parsedObject.time.create_timestamp;
        }
        //update last seen timestamp:
        analysis.last_seen_devices[parsedObject.device_id] =
                parsedObject.time.create_timestamp;
        //potentially could be a problem if create time > send time
        if(parsedObject.time.create_timestamp > parsedObject.time.send_timestamp) {
            console.error("umm");
        }

        return parsedObject;
    }).then(function (parsedObject) {
        //OS counting:
        //same data can also be fetched from sender_info
        if(_.has(parsedObject, "device.operating_system.kind")) {
            if(parsedObject.device.operating_system.kind.toLowerCase().match("android")){
                analysis.device_type_count.android+=1;
            } else if(parsedObject.device.operating_system.kind.toLowerCase().match("ios")){
                analysis.device_type_count.ios+=1;
            } else {
                analysis.device_type_count.other+=1;
            }
        }
        return parsedObject;
    }).then(function (parsedObject) {
        //country counting:
        //HUOM! some country fields might not be populated
        if(_.has(parsedObject, "sender_info.geo.country")){
            if(!_.has(analysis, "visitor_origin."+parsedObject.sender_info.geo.country)) {
                analysis.visitor_origin[parsedObject.sender_info.geo.country] = {
                    aggregate : 1
                };
                if(!_.has(parsedObject, "sender_info.geo.city")) {
                    analysis.visitor_origin[parsedObject.sender_info.geo.country][parsedObject.sender_info.geo.city] = 1;
                }
            } else {
                //country already exist
                analysis.visitor_origin[parsedObject.sender_info.geo.country].aggregate += 1;
                if(!_.has(parsedObject, "sender_info.geo.city")) {
                    analysis.visitor_origin[parsedObject.sender_info.geo.country][parsedObject.sender_info.geo.city] = 1;
                } else {
                    analysis.visitor_origin[parsedObject.sender_info.geo.country][parsedObject.sender_info.geo.city] += 1;
                }
            }

        } else {
            //direct to unknown:
            analysis.visitor_origin.unknown += 1;
        }
        return parsedObject;
    });
}).then(function () {
    //debug: display
    //console.log(dio(analysis, "  "));
});



////  file: pebble-js-app.js
//
//// change this token for your project
//var setPebbleToken = 'Y29N';
//
//
//Pebble.addEventListener('ready', function(e) 
//{
//	console.log("My app has started - Doing stuff...");
//	Pebble.showSimpleNotificationOnPebble("digilog", "I am running!");
//});
//
//Pebble.addEventListener('appmessage', function(e) 
//{
//	var key = e.payload.action;
//	if (typeof(key) != 'undefined') 
//	{
//		var settings = localStorage.getItem(setPebbleToken);
//		if (typeof(settings) == 'string') 
//		{
//			//try 
//			//{
//				Pebble.sendAppMessage(JSON.parse(settings));
//			//} 
//			//catch (e) 
//			//{
//			//}
//		}
//		var request = new XMLHttpRequest();
//		request.open('GET', 'http://x.SetPebble.com/api/' + setPebbleToken + '/' + Pebble.getAccountToken(), true);
//		request.onload = function(e) 
//		{
//			if (request.readyState == 4)
//			{
//				if (request.status == 200)
//				{
//					//try 
//					//{
//						Pebble.sendAppMessage(JSON.parse(request.responseText));
//					//}
//					//catch (e) 
//					//{
//					//}
//				}
//			}
//		};
//		request.send(null);
//	}
//});
//
//Pebble.addEventListener('showConfiguration', function(e) 
//{
//	Pebble.openURL('http://x.SetPebble.com/' + setPebbleToken + '/' + Pebble.getAccountToken());
//});
//
//Pebble.addEventListener('webviewclosed', function(e) 
//{
//	if ((typeof(e.response) == 'string') && (e.response.length > 0)) 
//	{
//		//try 
//		//{
//			Pebble.sendAppMessage(JSON.parse(e.response));
//			localStorage.setItem(setPebbleToken, e.response);
//		//}
//		//catch(e)
//		//{			
//		//}
//	}
//});
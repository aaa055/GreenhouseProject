//---------------------------------------------------------------------------
// name - имя cookie
// value - значение cookie
// [expires] - дата окончания действия cookie (по умолчанию - до конца сессии)
// [path] - путь, для которого cookie действительно (по умолчанию - документ, в котором значение было установлено)
// [domain] - домен, для которого cookie действительно (по умолчанию - домен, в котором значение было установлено)
// [secure] - логическое значение, показывающее требуется ли защищенная передача значения cookie
var caution = false;
function setCookie(name, value, expires, path, domain, secure) 
{
        var curCookie = name + "=" + escape(value) +
                ((expires) ? "; expires=" + expires.toGMTString() : "") +
                ((path) ? "; path=" + path : "") +
                ((domain) ? "; domain=" + domain : "") +
                ((secure) ? "; secure" : "")
        if (!caution || (name + "=" + escape(value)).length <= 4000)
                document.cookie = curCookie
        else
                if (confirm("Cookie превышает 4KB и будет вырезан !"))
                        document.cookie = curCookie
}
//-------------------------------------------------------------------------------------------------
// name - имя считываемого cookie
function getCookie(name) 
{
        var prefix = name + "="
        var cookieStartIndex = document.cookie.indexOf(prefix)
        if (cookieStartIndex == -1)
                return null
        var cookieEndIndex = document.cookie.indexOf(";", cookieStartIndex + prefix.length)
        if (cookieEndIndex == -1)
                cookieEndIndex = document.cookie.length
        return unescape(document.cookie.substring(cookieStartIndex + prefix.length, cookieEndIndex))
}
//-------------------------------------------------------------------------------------------------
// name - имя cookie
// [path] - путь, для которого cookie действительно
// [domain] - домен, для которого cookie действительно
function deleteCookie(name, path, domain) 
{
        if (getCookie(name)) 
        {
                document.cookie = name + "=" + 
                ((path) ? "; path=" + path : "") +
                ((domain) ? "; domain=" + domain : "") +
                "; expires=Thu, 01-Jan-70 00:00:01 GMT"
        }
}
//-------------------------------------------------------------------------------------------------
function  colorize_tables()
{
 $("tr:nth-child(odd) > td.colorize").addClass("odd");
}
//---------------------------------------------------------------------------
function prevent_default(e)
{
  if(!e)
    return;


  if(e.stopPropagation) e.stopPropagation();
  else e.cancelBubble = true;
  
  if(e.preventDefault) e.preventDefault();
  else e.returnValue = false;
    
    
}
//---------------------------------------------------------------------------
function OnlyDigits(event,elem) 
{

if(!event)
  event = window.event;


	if(event)
	{
    var work = event.charCode || event.keyCode;
    
		if ((work < 45 || work> 57) ) 
		{
			if( !(work == 8 || work == 9 || work == 13) ) // backspace, enter and tab keys (for FireFox)
				prevent_default(event);
		}
      
	} // if(event)

}
//---------------------------------------------------------------------------
window.onload = function() // раскрашиваем таблицы
{
	colorize_tables();
}
//---------------------------------------------------------------------------
String.prototype.trim = function() {
	return this.replace(/^\s+|\s+$/g,"");
}
String.prototype.ltrim = function() {
	return this.replace(/^\s+/,"");
}
String.prototype.rtrim = function() {
	return this.replace(/\s+$/,"");
}
//---------------------------------------------------------------------------
function isEmail(str) 
{
var supported = 0;

	if (window.RegExp) 
	{
		var tempStr = "a";
		var tempReg = new RegExp(tempStr);

		if (tempReg.test(tempStr)) 
		supported = 1;
	}


	if (!supported) 
		return (str.indexOf(".") > 2) && (str.indexOf("@") > 0);

	var r1 = new RegExp("(@.*@)|(\\.\\.)|(@\\.)|(^\\.)");
		var r2 = new RegExp("^.+\\@(\\[?)[a-zA-Z0-9\\-\\.]+\\.([a-zA-Z]{2,3}|[0-9]{1,3})(\\]?)$");

	return (!r1.test(str) && r2.test(str));
} 

function getE(id)
{
	return document.getElementById(id);
}

function inlineAlert(elemId)
{
	var e = getE(elemId);
	if(e)
		alert(e.innerHTML);
	else
		alert("Unknown error!");
}

function inlineConfirm(eId)
{
  var e = getE(eId);
	if(e)
		return confirm(e.innerHTML);
	else
	{
		alert("Unknown error!");
		return false;
	}
}

function showElem(elemId, visible)
{
 var elem = getE(elemId);


 if(elem != null)
 {
  var cs = elem.runtimeStyle;

  if(cs == null)
    cs = elem.style; // for Opera

  if(cs == null)
	 return;

  if(visible)
    cs.display = "block";
  else
    cs.display = "none";
  

 } // if(elem != null)

} // function
function get_left(e)
{
  var nLeftPos = e.offsetLeft;
    var eParElement = e.offsetParent;
    while (eParElement != null)
    {
        nLeftPos += eParElement.offsetLeft;
        eParElement = eParElement.offsetParent;
    }
    return nLeftPos;
}

function get_top(e)
{
    var nTopPos = e.offsetTop;
    var eParElement = e.offsetParent;
    while (eParElement != null)
    {
        nTopPos += eParElement.offsetTop;
        eParElement = eParElement.offsetParent;
    }
    return nTopPos;
}

function openWin(url,w,h)
{
  var wleft = (screen.width - w) / 2;
  var wtop = (screen.height - h) / 2;
  var w = window.open (url,"prev","location=no,status=no,scrollbars=1,menubar=no,resizable=no,width=" + w + ",height=" + h + ",left=" + wleft + ",top=" + wtop);
 // w.resizeTo(w, h);
 // w.moveTo(wleft, wtop);
  w.focus(); 
  return w;
}

// Set the "outer" HTML of an element.
function setOuterHTML(element, toValue)
{
	if (typeof(element.outerHTML) != 'undefined')
		element.outerHTML = toValue;
	else
	{
		var range = document.createRange();
		range.setStartBefore(element);
		element.parentNode.replaceChild(range.createContextualFragment(toValue), element);
	}
}



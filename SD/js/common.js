function prevent_default(e)
{
  if(!e)
    return;


  if(e.stopPropagation) e.stopPropagation();
  else e.cancelBubble = true;
  
  if(e.preventDefault) e.preventDefault();
  else e.returnValue = false;
    
    
}
function OnlyDigits(event,elem) 
{

if(!event)
  event = window.event;


	if(event)
	{
    var work = event.charCode || event.keyCode;
    
		if ((work < 45 || work> 57) ) 
		{
			if( !(work == 8 || work == 9 || work == 13) )
				prevent_default(event);
		}
      
	} // if(event)

}
String.prototype.trim = function() {
	return this.replace(/^\s+|\s+$/g,"");
}
String.prototype.ltrim = function() {
	return this.replace(/^\s+/,"");
}
String.prototype.rtrim = function() {
	return this.replace(/\s+$/,"");
}
function getE(id)
{
	return document.getElementById(id);
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
  

 }

}



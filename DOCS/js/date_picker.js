//---------------------------------------------------------------------------
// common language variables
//---------------------------------------------------------------------------
// english translation
//---------------------------------------------------------------------------
var arr_1_en = new Array('Sunday','Monday','Tuesday','Wednesday','Thursday','Friday','Saturday','Sun','Mon','Tue','Wed','Thu','Fri','Sat');
var arr_2_en = new Array('January','February','March','April','May','June','July','August','September','October','November','December','Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec');
var arr_3_en = new Array('Su','Mo','Tu','We','Th','Fr','Sa','Su','Mo','Tu','We','Th','Fr','Sa');
var arr_4_en = new Array('Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec');
var close_btn_text_en = 'Close';
var prev_year_alt_en = 'Go to previous year';
var prev_month_alt_en = 'Go to previous month';
var next_month_alt_en = 'Go to next month';
var next_year_alt_en = 'Go to next year';
//---------------------------------------------------------------------------
// russian translation
//---------------------------------------------------------------------------
var arr_1_ru = new Array('Воскресенье','Понедельник','Вторник','Среда','Четверг','Пятница','Суббота','Вс','Пн','Вт','Ср','Чт','Пт','Сб');
var arr_2_ru = new Array('Январь','Февраль','Март','Апрель','Май','Июнь','Июль','Август','Сентябрь','Октябрь','Ноябрь','Декабрь','Янв','Фев','Мар','Апр','Май','Июн','Июл','Авг','Сен','Окт','Ноя','Дек');
var arr_3_ru = new Array('Вс','Пн','Вт','Ср','Чт','Пт','Сб','Вс','Пн','Вт','Ср','Чт','Пт','Сб');
var arr_4_ru = new Array('Янв','Фев','Мар','Апр','Май','Июн','Июл','Авг','Сен','Окт','Ноя','Дек');
var close_btn_text_ru = 'Закрыть';
var prev_year_alt_ru = 'Перейти к предыдущему году';
var prev_month_alt_ru = 'Перейти к предыдущему месяцу';
var next_month_alt_ru = 'Перейти к следующему месяцу';
var next_year_alt_ru = 'Перейти к следующему году';
//---------------------------------------------------------------------------
function DatePicker(lang,id,isPopup,targetElement,date_format,scrollable_container,from_today_only)
{

  this.ID = id;
  this.IsPopup = isPopup;
  this.TargetElement = targetElement;
  this.ScrollContainer = scrollable_container;
  this.Visible = false;
  this.Table = null;
  this.Container = null;
  this.MouseInControl = false;
  this.CurrentDate = null;
  this.Language = lang;
  this.Today = 0;
  this.SelectedCell = null;
  this.DateFormat = date_format;
  
  this.MouseOverTD = null;
  this.MouseOverTD = null;
  this.SavedOverClassName = '';
  this.FromTodayOnly = false;
  if(from_today_only)
	this.FromTodayOnly = from_today_only;
  
  
  this.Init(); // init common variables
  this.Create(); // create table
  
  if(this.IsPopup)
    this.AsPopup();
  else
    this.AsInline();
    
    
  return this;
}
//---------------------------------------------------------------------------
DatePicker.prototype.AdjustFirstDay = function(lang)
{
  switch(lang)
  {
    case 'en':
        this.FirstDay = 0; // Sunday first 
    break;
      
    case 'ru':
      this.FirstDay = 1; // Monday first 
    break;
  }
  
  return this;
}
//---------------------------------------------------------------------------
DatePicker.prototype.ach = function(to,elem)
{
  to.appendChild(elem);
  
  return this;
}
//---------------------------------------------------------------------------
DatePicker.prototype.get_left = function(e)
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
//---------------------------------------------------------------------------
DatePicker.prototype.get_top = function(e)
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
//---------------------------------------------------------------------------
DatePicker.prototype.sc = function (e,cName)
{
	e.setAttribute('class',cName);
	e.setAttribute('className',cName);

	return this;
};
//-----------------------------------------------------------------------------
DatePicker.prototype.AsPopup = function()
{

 
  if(this.TargetElement.type.toLowerCase()  != 'text')
  {
    alert("Target should be <input type='text'>!");
    return this;
  }
  
  
		this.Table.style.position = 'absolute';
		this.EnsurePosition();
		
		
		this.ach(document.body,this.Table);
		this.TargetElement.datePicker = this;
		this.TargetElement.onfocus = function () {this.datePicker.Show();};
		this.TargetElement.onblur = function () {if(!this.datePicker.MouseInControl){this.datePicker.Hide();}};
  

  return this;
}
//---------------------------------------------------------------------------
DatePicker.prototype.EnsurePosition = function()
{
	if(!this.IsPopup)
		return;
	
	
	if(this.ScrollContainer)
	{
    var tp = this.get_top(this.TargetElement);
    var to = this.get_top(this.ScrollContainer);
    var scr_left = this.get_left(this.ScrollContainer);
    
    this.Table.style.top =  to + (tp - to) - this.ScrollContainer.scrollTop + this.TargetElement.offsetHeight;
    this.Table.style.left =  (this.get_left(this.TargetElement) - this.ScrollContainer.scrollLeft);// + scr_left;
    
    
	}
	else
	{
	var left = this.get_left(this.TargetElement);
	var top = this.get_top(this.TargetElement) + this.TargetElement.offsetHeight;
	this.Table.style.top =  top + 'px';
	this.Table.style.left = left + 'px';
 }
		
}
//---------------------------------------------------------------------------
DatePicker.prototype.AsInline = function()
{
  this.Container = this.TargetElement;
  
  this.ach(this.Container, this.Table);
  
  this.Show(); // ensure visible
  
  return this;
}
//---------------------------------------------------------------------------
DatePicker.prototype.IsLeapYear = function(year)
{
 if (year % 4 == 0) 
  {
    if (year % 100 == 0) 
      return (year % 400 == 0);
    else 
      return true;
  } 
 else
  return false;
}
//---------------------------------------------------------------------------
DatePicker.prototype.CreateDayNames = function()
{
  return eval('arr_1_' + this.Language);
}
//---------------------------------------------------------------------------
DatePicker.prototype.CreateMonthNames = function()
{
  return eval('arr_2_' + this.Language);
}
//---------------------------------------------------------------------------
DatePicker.prototype.CreateDOF = function(lang)
{
  return eval('arr_3_' + this.Language);
}
//---------------------------------------------------------------------------
DatePicker.prototype.CreateMSN = function(lang)
{
  return eval('arr_4_' + this.Language);
}
//---------------------------------------------------------------------------
DatePicker.prototype.Init = function()
{
  this.CurrentDate = new Date();
  
  this.MonthToDisplay = this.CurrentDate.getMonth();
  this.YearToDisplay = this.CurrentDate.getFullYear();  
  
  var isLeap = this.IsLeapYear(this.CurrentDate.getFullYear());
  
  this.MonthsDays = new Array(31,(isLeap ?  29 : 28),31,30,31,30,31,31,30,31,30,31);

  this.AdjustFirstDay(this.Language);
	this.DaysOfWeek = this.CreateDOF(this.Language);
	this.MonthsShortNames = this.CreateMSN(this.Language);
  
  return this;
}
//---------------------------------------------------------------------------
DatePicker.prototype.ce = function(tag)
{
  return document.createElement(tag);
}
//---------------------------------------------------------------------------
DatePicker.prototype.getE = function(id)
{
  return document.getElementById(id);
}
//---------------------------------------------------------------------------
DatePicker.prototype.sa = function(e,name,val)
{
  e.setAttribute(name,val);
  return this;
}
//---------------------------------------------------------------------------
DatePicker.prototype.Create = function()
{

  this.Table = this.ce('TABLE');
 	this.sa(this.Table,'id',this.ID);
 	
 	this.sc(this.Table,'date_picker');
 	
 	//this.Table.style.width = '200px';

	this.Table.onselectstart = function() {return false;};
	this.Table.ondrag = function() { return false; };

  var tbody = this.ce('TBODY');

  // create header
	var tr = this.ce('TR');
	var td = this.ce('TD');
	
	var hdr = this.CreateHeader();
	
	this.ach(td,hdr);
	this.ach(tr,td);
	this.ach(tbody,tr);
	
	// create week days cells
	tr = this.ce('TR');
	td = this.ce('TD');

	this.CreateWeekDays(td);
	this.ach(tr,td);
	this.ach(tbody,tr);
	
	// create month days cells
  tr = this.ce('TR');
	td = this.ce('TD');

	this.CreateMonthDays(td);
	this.ach(tr,td);
	this.ach(tbody,tr);


  // create footer
  if(this.IsPopup) // create footer only if in popup mode !!!
  {
    tr = this.ce('TR');
    td = this.ce('TD');

    var footer = this.CreateFooter();
    this.ach(td,footer);
    this.ach(tr,td);
    this.ach(tbody,tr);
    
  } // if(this.IsPopup)

  // append tbody to table
  this.ach(this.Table,tbody);
  
	this.Table.Owner = this;
	this.Table.onmouseover = function() {this.Owner.MouseInControl = true;};
	this.Table.onmouseout = function() {this.Owner.MouseInControl = false;};
  
  
  this.Hide();
  
	return this;
}
//---------------------------------------------------------------------------
DatePicker.prototype.ctn = function(text)
{
  return document.createTextNode(text);
}
//---------------------------------------------------------------------------
DatePicker.prototype.DeleteMonthDays = function()
{
	this.Cells = new Array();
	this.DaysContainer.removeChild(this.DaysContainer.firstChild);

}
//---------------------------------------------------------------------------
DatePicker.prototype.SelectYearMonth = function(year,month)
{


  this.MonthToDisplay = month;
  this.YearToDisplay = year;

  this.DeleteMonthDays();
  this.CreateMonthDays(this.DaysContainer);
  
  this.TargetElement.focus();
  
}
//---------------------------------------------------------------------------
DatePicker.prototype.CreateMonthDays = function(td_elem)
{

  this.SelectedCell = null;
  this.DaysContainer = td_elem;

  this.MonthDaysTable = this.ce('TABLE');
  this.sa(this.MonthDaysTable,'id',this.ID + 'month_days');
  this.sc(this.MonthDaysTable,'month_days');

  this.Cells = new Array();
  
  var tbody,tr,td;
  tr = null;
  
  tbody = this.ce('TBODY');
  
  var cntr = 0;
  var dt = new Date();
  
  this.MonthsDays[1] = this.IsLeapYear(this.YearToDisplay) ? 29 : 28;
  
  
  this.Today = dt.getDate();

  var beginDate = new Date(this.YearToDisplay,this.MonthToDisplay,1);
  var fDay = beginDate.getDay();
  
  var cnt_days = this.MonthsDays[this.MonthToDisplay];
  
	var endDate = new Date(this.YearToDisplay,this.YearToDisplay,cnt_days);
	dt = new Date(beginDate);
	dt.setDate(dt.getDate() + (this.FirstDay - beginDate.getDay()) - (this.FirstDay - beginDate.getDay() > 0 ? 7 : 0) );

  var start_found = false;
  var end_found = false;
  
  var curDate = new Date();
  var cdYear = curDate.getFullYear();
  var cdMonth = curDate.getMonth();
  var cdDay =  curDate.getDate();
  curDate = new Date(cdYear,cdMonth,cdDay);
  
 
  var cntCells = 42;
  for(var i=0;i<cntCells;i++)
  {
    if(!(i%7))
    {
      if(tr)
        this.ach(tbody,tr);
        
      if(end_found &&  ((i+7) == cntCells || (fDay == this.FirstDay) )) // don't need the empty rows
      {
        tr = null;
        break;
      }


      tr = this.ce('TR');
    } // if
    
      var td = this.ce('TD');
      
//      alert(dt + '\n' + curDate);

      if(dt < curDate)
		this.sc(td,'month_day_in_past');
	  else
		this.sc(td,'month_day');

      var daynum = dt.getDate();
      var display_val = daynum;
      
      if(daynum == 1)
        start_found = true;

      
      if(!start_found || end_found)
        display_val = '';
        
      
      this.ach(td,this.ctn(display_val));
      this.ach(tr,td);

      if(start_found && !end_found)
      {
        var cell = new DayCell(this,td,tr,dt,curDate,this.FromTodayOnly);

        this.Cells.push(cell);
        td.cellObj = cell;
      }
          
      dt.setDate(dt.getDate()+1);

      if(start_found && daynum >= cnt_days)
        end_found = true;
        
    
  } // for
  
  if(tr)
    this.ach(tbody,tr);
  

  this.ach(this.MonthDaysTable,tbody);
  this.ach(td_elem,this.MonthDaysTable);
  
  this.SetCurrentDateDisplay();

  return this;
}
//---------------------------------------------------------------------------
DatePicker.prototype.CreateWeekDays = function(td_elem)
{
  
  this.WeekDaysTable = this.ce('TABLE');
  this.sa(this.WeekDaysTable,'id',this.ID + 'days_of_week');
  this.sc(this.WeekDaysTable,'days_of_week');
  
  var tbody,tr,td;
  
  tbody = this.ce('TBODY');
 
  tr = this.ce('TR');
  
  for(var i=0;i<7;i++)
  {
    td = this.ce('TD');
    this.sa(td,'id',this.ID + 'day_of_week');
    this.sc(td,'day_of_week');
    
    this.ach(td,this.ctn(this.DaysOfWeek[i+ this.FirstDay]));
    
    this.ach(tr,td);
    
  } // for
  
  
  this.ach(tbody,tr);
  
  
  this.ach(this.WeekDaysTable,tbody);
  this.ach(td_elem,this.WeekDaysTable);
  

  return this;
}
//---------------------------------------------------------------------------
DatePicker.prototype.GoToPrevYear = function()
{
  var owner = this.owner;
  var ytd = owner.YearToDisplay;
  --ytd;
  owner.SelectYearMonth(ytd,owner.MonthToDisplay);
  
}
//---------------------------------------------------------------------------
DatePicker.prototype.GoToNextYear = function()
{
  var owner = this.owner;
  var ytd = owner.YearToDisplay;
  ++ytd;
  owner.SelectYearMonth(ytd,owner.MonthToDisplay);
  
}
//---------------------------------------------------------------------------
DatePicker.prototype.GoToPrevMonth = function()
{
  var owner = this.owner;
  var ytd = owner.YearToDisplay;
  var mtd = owner.MonthToDisplay;
  
  --mtd;
  if(mtd < 0)
  {
    mtd = 11;
    --ytd;
  }
   
  owner.SelectYearMonth(ytd,mtd);
  
}
//---------------------------------------------------------------------------
DatePicker.prototype.GoToNextMonth = function()
{
  var owner = this.owner;
  var ytd = owner.YearToDisplay;
  var mtd = owner.MonthToDisplay;
  
  ++mtd;
  if(mtd > 11)
  {
    mtd = 0;
    ++ytd;
  }
   
  owner.SelectYearMonth(ytd,mtd);
  
}
//---------------------------------------------------------------------------
DatePicker.prototype.CreateHeader = function()
{
  var div = this.ce('DIV');
  this.sa(div,'id',this.ID + 'header');
  this.sc(div,'header');
  
  this.HeaderTable = this.ce('TABLE');
  this.sc(this.HeaderTable,'header');
  
  var tbody, tr,td, a;
  
  tbody = this.ce('TBODY');
  
  tr = this.ce('TR');
  
  td = this.ce('TD');
  this.sc(td,'prev_year');
  
  a = this.ce('A');
  this.sc(a,'prev_year');
  this.ach(a,this.ctn('<<'));
  a.owner = this;
  a.onclick = this.GoToPrevYear;
  var hint = eval('prev_year_alt_' + this.Language);
  this.sa(a,'title',hint);
  this.sa(a,'alt',hint);
  
  
  this.ach(td,a);
  this.ach(tr,td);

  td = this.ce('TD');
  this.sc(td,'prev_month');
  
  a = this.ce('A');
  this.sc(a,'prev_month');
  this.ach(a,this.ctn('<'));
  a.owner = this;
  a.onclick = this.GoToPrevMonth;
  hint = eval('prev_month_alt_' + this.Language);
  this.sa(a,'title',hint);
  this.sa(a,'alt',hint);
  
  this.ach(td,a);
  this.ach(tr,td);
  
  // current date holder
  td = this.ce('TD');
  this.CurrentDateDisplay = td;
  this.sc(td,'current_date');
  this.ach(td,this.ctn(''));
  this.ach(tr,td);
    

  td = this.ce('TD');
  this.sc(td,'next_month');
  a = this.ce('A');
  this.sc(a,'next_month');
  this.ach(a,this.ctn('>'));
  a.owner = this;
  a.onclick = this.GoToNextMonth;
  hint = eval('next_month_alt_' + this.Language);
  this.sa(a,'title',hint);
  this.sa(a,'alt',hint);
  
  this.ach(td,a);
  this.ach(tr,td);

  td = this.ce('TD');
  this.sc(td,'next_year');
  a = this.ce('A');
  this.sc(a,'next_year');
  this.ach(a,this.ctn('>>'));
  a.owner = this;
  a.onclick = this.GoToNextYear;
  hint = eval('next_year_alt_' + this.Language);
  this.sa(a,'title',hint);
  this.sa(a,'alt',hint);
  
  this.ach(td,a);
  this.ach(tr,td);

  
  this.ach(tbody,tr);
  
  
  this.ach(this.HeaderTable,tbody);
  this.ach(div,this.HeaderTable);
  
  //this.ach(div,this.ctn('header will be here'));
  
  return div;
}
//---------------------------------------------------------------------------
DatePicker.prototype.SetCurrentDateDisplay = function()
{
  var d = new Date(this.YearToDisplay,this.MonthToDisplay,1);
  this.CurrentDateDisplay.removeChild(this.CurrentDateDisplay.firstChild);
  
  var res = d.dateFormat('F Y',this);
  
  var nobr = this.ce('NOBR');
  this.ach(nobr,this.ctn(res));
  
  this.ach(this.CurrentDateDisplay, nobr);

   
}
//---------------------------------------------------------------------------
DatePicker.prototype.CreateFooter = function()
{
  var div = this.ce('DIV');
  this.sa(div,'id',this.ID + 'footer');
  this.sc(div,'footer');
  
  var span = this.ce('SPAN');
  this.sa(span,'id',this.ID + 'span_footer');
  this.sc(span,'footer');
  
  this.ach(span,this.ctn(eval('close_btn_text_' + this.Language)));
  
  this.ach(div,span);
  
  span.Owner = this;
  span.onclick = this.close_click;
  
  return div;
}
//---------------------------------------------------------------------------
DatePicker.prototype.close_click = function()
{
  var owner = this.Owner;
  owner.Hide();
}
//---------------------------------------------------------------------------
DatePicker.prototype.Show = function()
{
	this.EnsurePosition();

	this.Table.style.display = 'block';
	this.Visible = true;
	
	
	return this;
  
}
//---------------------------------------------------------------------------
DatePicker.prototype.Hide = function()
{
	this.Table.style.display = 'none';
	this.Visible = false;
  
	return this;
}
//---------------------------------------------------------------------------
DatePicker.prototype.Repaint = function()
{
  
	for(var i=0;i<this.Cells.length;i++) 
	{
		this.Cells[i].selected = false;
	}  
	/*
	var desired_idx = 0;
	
	if(this.SelectedCell != null)
	{
    desired_idx = this.SelectedCell.parentNode.rowIndex + '_' + this.SelectedCell.cellIndex;
  }
	*/
	
	for(var i=0;i<this.Cells.length;i++)
	{
   // var cur_idx = this.Cells[i].TableRow.rowIndex + '_' + this.Cells[i].TableCell.cellIndex;
    if(this.Cells[i].TableCell ==  this.SelectedCell)
			//	if(desired_idx ==  cur_idx)
				{
          this.Cells[i].selected = true;
        }

		this.Cells[i].setClass();
	}	
	return this;
}
//---------------------------------------------------------------------------
DatePicker.prototype.Toggle = function()
{
  if(this.Visible)
    this.Hide();
  else
    this.Show();
  
	return this;
}
//---------------------------------------------------------------------------
function DayCell(owner,td,tr,date,curDate,ftdo)
{
  this.Owner = owner;
  this.TableCell = td;
  this.TableRow = tr;
  this.Date = new Date(date);
  this.CurDate = new Date(curDate);
  this.InPast = this.Date < this.CurDate;
  if(!ftdo)
	this.InPast = false;
  
	//assign the event handlers for the table cell element
	this.TableCell.onclick = this.onclick;
	this.TableCell.onmouseover = this.onmouseover;
	this.TableCell.onmouseout = this.onmouseout;
	
	//and set the CSS class of the table cell
	this.setClass();
  
  return this;
}
//---------------------------------------------------------------------------
DayCell.prototype.onmouseover = function () 
{
  if(!this.cellObj.InPast)
	this.cellObj.Owner.sc(this,'month_day_over');
};
//-----------------------------------------------------------------------------
DayCell.prototype.onmouseout = function ()
{
	if(!this.cellObj.InPast)
		this.cellObj.setClass();
};
//-----------------------------------------------------------------------------
DayCell.prototype.onclick = function () 
{


	var cell = this.cellObj;
	var owner = cell.Owner;
	
	if(cell.InPast)
		return;
	
	////
/*	
	owner.MonthToDisplay++;
	if(owner.MonthToDisplay > 11)
    owner.MonthToDisplay = 0;

	owner.SelectYearMonth(owner.YearToDisplay,owner.MonthToDisplay);
*/    
 ///   
    
    
	
	
	owner.SelectedCell = this;
	
	owner.Repaint();
	
	if(owner.TargetElement)
	{
		owner.TargetElement.value = cell.Date.dateFormat(owner.DateFormat,owner);
		
		var oAttribs = owner.TargetElement.attributes;
		for (var i = 0; i < oAttribs.length; i++)
		{
			var oAttrib = oAttribs[i];
			if(oAttrib.nodeName == 'ondatechange')
			{
				eval(oAttrib.nodeValue);
				break;
			}
		} // for
	}
	
	if(owner.IsPopup)
	{
    owner.Hide();
  }
	
}
//-----------------------------------------------------------------------------
DayCell.prototype.setClass = function ()
{

	if(this.InPast)
	{
		this.cellClass = 'month_day_in_past';
		this.Owner.sc(this.TableCell,this.cellClass);
		return this;
	}

	if(this.selected) {
		this.cellClass = 'month_day_selected';
	}
	/*
	else if(this.owner.displayMonth != this.date.getMonth() ) 
	{
		this.cellClass = 'notmnth';	
	}
	*/
	else if(this.Date.getDay() > 0 && this.Date.getDay() < 6) {
		this.cellClass = 'month_day';
	}
	else {
		this.cellClass = 'month_day_weekend';
	}
	
	if(this.Date.getFullYear() == this.Owner.CurrentDate.getFullYear() 
	&& this.Date.getMonth() == this.Owner.CurrentDate.getMonth() 
	&& this.Date.getDate() == this.Owner.CurrentDate.getDate()) 
	{
		this.cellClass = 'month_day_today';
	}

	 this.Owner.sc(this.TableCell,this.cellClass);
	 
	 return this;
}
//---------------------------------------------------------------------------
Date.prototype.dateFormat = function(format,owner)
{

	if(!format) { // the default date format to use - can be customized to the current locale
		format = 'm/d/Y';
	}
	
	if(!this.getFullYear)
		return "";
	
	var initDt = new Date(0,0,0,0,0,0,0);
	
	
	LZ = function(x) {return(x < 0 || x > 9 ? '' : '0') + x};
	var MONTH_NAMES = null;
	if(owner)
    MONTH_NAMES = owner.CreateMonthNames(); 
    
	var DAY_NAMES = null;
	if(owner)
    DAY_NAMES = owner.CreateDayNames(); 
    
	format = format + "";
	var result="";
	var i_format=0;
	var c="";
	var token="";
	var y=this.getFullYear().toString();
	var M=this.getMonth()+1;
	var d=this.getDate();
	var E=this.getDay();
	var H=this.getHours();
	var m=this.getMinutes();
	var s=this.getSeconds();
	
	if(initDt.getFullYear().toString() == y &&
      initDt.getMonth()+1 ==  M &&
      initDt.getDate() == d &&
      initDt.getDay() == E &&
      initDt.getHours() == H &&
      initDt.getMinutes() == m &&
      initDt.getSeconds() == s)
      {
        // empty date
         y = '0000';
         M = 0;
         d = 0;
         E = 0;
         H = 0;
         m = 0;
         s = 0;
         
      } // if
	
	var yyyy,yy,MMM,MM,dd,hh,h,mm,ss,ampm,HH,H,KK,K,kk,k;
	// Convert real this parts into formatted versions
	var value = new Object();
	//if (y.length < 4) {y=''+(y-0+1900);}
	value['Y'] = y.toString();
	value['y'] = y.substring(2);
	value['n'] = M;
	value['m'] = LZ(M);
	value['F'] = MONTH_NAMES ? MONTH_NAMES[M-1] : '';
	value['M'] = MONTH_NAMES ? MONTH_NAMES[M+11] : '';
	value['j'] = d;
	value['d'] = LZ(d);
	value['D'] = DAY_NAMES ? DAY_NAMES[E+7] : '';
	value['l'] = DAY_NAMES ? DAY_NAMES[E] : '';
	value['G'] = H;
	value['H'] = LZ(H);
	if (H==0) {value['g']=12;}
	else if (H>12){value['g']=H-12;}
	else {value['g']=H;}
	value['h']=LZ(value['g']);
	if (H > 11) {value['a']='pm'; value['A'] = 'PM';}
	else { value['a']='am'; value['A'] = 'AM';}
	value['i']=LZ(m);
	value['s']=LZ(s);
	//construct the result string
	while (i_format < format.length) {
		c=format.charAt(i_format);
		token="";
		while ((format.charAt(i_format)==c) && (i_format < format.length)) {
			token += format.charAt(i_format++);
			}
		if (value[token] != null) { result=result + value[token]; }
		else { result=result + token; }
		}
	return result;
}
//---------------------------------------------------------------------------

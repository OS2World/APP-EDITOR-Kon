// These are som tests I have run during development. I include it for your
// amusement or perhaps you have use for some of it...

loop = 0;

columnDescription = [];

addIndex()
{
  index = 1;
  while( findString( "NULL, NULL );", "" ) )
  {
    charLeft();
    charLeft();
    charLeft();
    insertString( ", " @ index );
    index++;
  }
}

// Overload test

hej()
{
  print( "Now in first instance!" );
}

hej()
{
  print( "Calling first instance!" );
  :hej();
  print( "First instance was called!" );
}

hej()
{
  print( "Calling second instance!" );
  :hej();
  print( "Second instance was called!" );
}

fib( n )
{
  if( n == 0 )
    return 0;

  if( n == 1 )
    return 1;

  if( !(++loop % 500) ) print( loop );

  return fib(n-1) + fib(n-2);
}

fibI( n )
{
  f = 1;
  newf = 0;
  lastf = 0;

  if( n == 1 ) return 1;

  for( a = 2; a <= n; ++a )
  {
    newf = f + lastf;
    lastf = f;
    f = newf;
  }

  return newf;
}

ffib( n )
{
  if( n == 0 )
    return 0;

  if( n == 1 )
    return 1;

  return ffib(n-1) + ffib(n-2);
}

noop(i)
{
}

testif()
{
  for( i = 0; i < 1000; i++ )
    noop(i);
    //resetMark();
    //length( "abc" );
}

array(a)
{
  a = a + [["a", "b"]];
  print( a[0] @ " " @ a[2][2] @ " " @ a[3][1] );
}

ta()
{
  for( i = 0; i < 500; i++ )
  {
    a = [1,2,[1,2,3]];
    array( a );
  }
}

test()
{
  print( "*** TEST" );

  testsort = [ 5,2,4,3,9,6,10,7,8,1 ];

  len = length( testsort );

  qsort( testsort, 0, len );

  str = "";

  for( i = 0; i < 10; i++ )
    str += testsort[i] + " ";

  print( str );

  loop = 0;

  print( "fib: " + fib( 18 ) );
}

mvtest()
{
  for( i = 0; i < 10; i++ )
    if( !lineDown() )
      print( "EOF" );
}

endTest()
{
  while( lineDown() );
}

startTest()
{
  while( lineUp() );
}

atest()
{
  loop=0;
  print( fib(12) );
}

up()
{
  lineUp();
}

insertAsteriskLine()
{
  insertLine();
  line = '';
  for( i = 0; i < 80; i++ )
    line += '*';
  insertString( line );
  insertLine();
}

hejtest()
{
  debug();
  c = [1,2] + [3] + [4,5];

  c += ["hejsan", "svejsan"];

  for( i = 0; i < 7; i++ )
    print( c[i] + ", " );

  a = c + ["hej igen"];

  for( i = 0; i < 8; i++ )
    print( a[i] + ", " );

  debug();

  a = 10;
  a += 20;
  print( a );
  debug();
}

test2()
{
  a = [7,84,6,5,687,56,5,67,5,76,56,7,567,56,71,3,34,42,34,52,23,3,223,32,23,23,23,23,23,23,24,4,12,21,24,34,3,52,3,2,523,23,3,63,17,184,16,15,1687,156,15,167,15,176,156,17,1567,156,171,13,134,142,134,152,123,13,1223,132,123,123,123,123,123,123,124,14,112,121,124,134,13,152,13,12,1523,123,13,163];
  qsort(a, 0, length(a));
  for(i = 0; i < length(a); i++ ) 
    print(a[i]);
}

stress()
{
  while( 1 )
  {
    insertString( "123456789012345678901234567890" );
    undo();
    toLineStart();
    while( lineDown() );
    while( lineUp() );
    insertString( "123456789012345678901234567890" );
    undo();
    toLineStart();
  }
}

stress2()
{
  while( 1 )
  {
    while( lineDown() );
    while( lineUp() );
  }
}

stress3()
{
  while( 1 )
    test2();
}

stress4()
{
  i = 0;
  while( 1 )
  {
    toLineStart();
    toLineEnd();
    print( i++ );
  }
}

addPos( &a, pos, info )
{
  columnDescription += 
    [[columnDescription[length(columnDescription)-1][0] + pos, info]];
  a += [a[length(a)-1]+pos];
}

transHuvudDelim()
{
  columnDescription = [[3, "Transaktionstyp"]]; 
  a = [3];
  addPos( a, 3, "Uppdragskod" );
  addPos( a, 10, "Transaktionsid" );
  addPos( a, 3, "Transaktionsid ref" );
  addPos( a, 1, "Ärendenr" );
  addPos( a, 255, "Meddelande" );
  addPos( a, 10, "Meddelandedatum" );
  addPos( a, 12, "Meddelandetid");

  return a;
}

kundInfoDelim()
{
  a = transHuvudDelim();

  addPos( a, 10, "" );
  addPos( a, 13, "" );
  addPos( a, 30, "" );
  addPos( a, 30, "" );
  addPos( a, 30, "" );
  addPos( a, 30, "" );
  addPos( a, 5, "" );
  addPos( a, 30, "" );
  addPos( a, 15, "" );
  addPos( a, 15, "" );
  addPos( a, 2, "" );
  addPos( a, 10, "" );
  addPos( a, 01, "" );
  addPos( a, 30, "" );
  addPos( a, 15, "" );
  addPos( a, 30, "" );

  setDelimitorLines( a );
}

kundInfoTransDelim()
{
  a = transHuvudDelim();

  addPos( a, 10, "KundId KLK" );
  addPos( a, 13, "KundpostId VA" );
  addPos( a, 30, "Namn KLK" );
  addPos( a, 30, "Fakturaadress" );
  addPos( a, 30, "C/O Adress" );
  addPos( a, 30, "Attention" );
  addPos( a, 5, "Postnr" );
  addPos( a, 30, "Ort" );
  addPos( a, 15, "Telefon1" );
  addPos( a, 15, "Telefon2" );
  addPos( a, 2, "Landkod" );
  addPos( a, 10, "Organisationsnr" );
  addPos( a, 30, "Namn VA" );

  setDelimitorLines( a );
}

aboInfoDelim()
{
  a = transHuvudDelim();

  addPos( a, 10, "" );
  addPos( a, 13, "" );
  addPos( a, 10, "" );
  addPos( a, 8, "" );
  addPos( a, 2, "" );
  addPos( a, 2, "" );
  addPos( a, 1, "" );
  addPos( a, 10, "" );
  addPos( a, 10, "" );
  addPos( a, 3, "" );
  addPos( a, 10, "" );
  addPos( a, 40, "" );
  addPos( a, 4, "" );
  addPos( a, 4, "" );
  addPos( a, 10, "" );
  addPos( a, 1, "" );
  addPos( a, 10, "" );
  addPos( a, 13, "" );
  addPos( a, 10, "" );
  addPos( a, 10, "" );
  addPos( a, 35, "" );
  addPos( a, 10, "" );
  addPos( a, 10, "" );
  addPos( a, 2, "" );
  addPos( a, 2, "" );
  addPos( a, 30, "" );
  addPos( a, 15, "" );
  addPos( a, 15, "" );
  addPos( a, 255, "" );
  addPos( a, 10, "" );
  addPos( a, 10, "" );
  addPos( a, 1, "" );
  addPos( a, 10, "" );
  addPos( a, 10, "" );
  addPos( a, 10, "" );
  addPos( a, 10, "" );

  setDelimitorLines( a );
}

aboInfoTransDelim()
{
  a = transHuvudDelim();

  addPos( a, 10, "Kundid KLK" );
  addPos( a, 13, "KundpostID VA" );
  addPos( a, 10, "AbonnemangsID KLK" );
  addPos( a, 8, "AbonnemangsID VA" );
  addPos( a, 2, "Abonnemang Typ" );
  addPos( a, 2, "Kategori" );
  addPos( a, 1, "Avtal självavläsarrabatt" );
  addPos( a, 10, "Startdatum" );
  addPos( a, 10, "Slutdatum" );
  addPos( a, 3, "Antal mätare" );
  addPos( a, 10, "Fnr" );
  addPos( a, 40, "Fastighetsbeteckning" );
  addPos( a, 4, "Fastighetsbeteckning" );
  addPos( a, 4, "Fastighetsbeteckning" );
  addPos( a, 10, "Faktureringsspärr" );
  addPos( a, 1, "Sepearat faktura" );
  addPos( a, 10, "Kundid KLK (fg)" );
  addPos( a, 13, "KundpostID VA (fg)" );
  addPos( a, 10, "Abonnemangsid KLK (fg)" );
  addPos( a, 10, "Datum ägarbyte" );
  addPos( a, 35, "Brukningsadress" );
  addPos( a, 10, "Första dag för fakturering" );
  addPos( a, 10, "Förslag till tidigaste debitering" );
  addPos( a, 2, "Förslag till antal deb. per år" );
  addPos( a, 30, "Kontaktperson" );
  addPos( a, 15, "Telefon 1" );
  addPos( a, 15, "Telefon 2" );
  addPos( a, 255, "Upplysning" );
  addPos( a, 10, "Vilandedatum fom" );
  addPos( a, 10, "Vilandedatum tom" );

  setDelimitorLines( a );
}

matarlistaDelim()
{
  a = transHuvudDelim();

  addPos( a, 10, "" );
  addPos( a, 13, "" );
  addPos( a, 10, "" );
  addPos( a, 8, "" );
  addPos( a, 3, "" );
  addPos( a, 10, "" );
  addPos( a, 5, "" );
  addPos( a, 5, "" );
  addPos( a, 2, "" );
  addPos( a, 1, "" );
  addPos( a, 40, "" );
  addPos( a, 1, "" );
  addPos( a, 10, "" );
  addPos( a, 10, "" );
  addPos( a, 10, "" );
  addPos( a, 10, "" );
  addPos( a, 10, "" );
  addPos( a, 1, "" );

  setDelimitorLines( a );
}

matarlistaTransDelim()
{
  a = transHuvudDelim();

  addPos( a, 10, "Kundid KLK" );
  addPos( a, 13, "Kundpostid VA" );
  addPos( a, 10, "Abonnemangsid KLK" );
  addPos( a, 8, "Abonnemangsid VA" );
  addPos( a, 3, "Mätarid VA" );
  addPos( a, 10, "Uppdragsnr KLK" );
  addPos( a, 5, "Storlek" );
  addPos( a, 5, "Debiterbar storlek" );
  addPos( a, 2, "Art" );
  addPos( a, 1, "Pulsavgift" );
  addPos( a, 40, "Mätarplatsadress" );
  addPos( a, 1, "Brandpost" );
  addPos( a, 10, "Uppdatum" );
  addPos( a, 10, "Neddatum" );
  addPos( a, 10, "Uppdragsnr KLK" );
  addPos( a, 10, "Mätarställning KLK" );
  addPos( a, 10, "Avläsningsdatum" );
  addPos( a, 1, "Avläsning önskas" );

  setDelimitorLines( a );
}

matarinfoDelim()
{
  a = transHuvudDelim();

  addPos( a, 10, "" );
  addPos( a, 13, "" );
  addPos( a, 10, "" );
  addPos( a, 8, "" );
  addPos( a, 3, "" );
  addPos( a, 10, "" );
  addPos( a, 5, "" );
  addPos( a, 1, "" );
  addPos( a, 8, "" );
  addPos( a, 10, "" );
  addPos( a, 12, "" );
  addPos( a, 1, "" );
  addPos( a, 8, "" );
  addPos( a, 10, "" );
  addPos( a, 1, "" );
  addPos( a, 7, "" );
  addPos( a, 7, "" );
  addPos( a, 1, "" );
  addPos( a, 10, "" );
  addPos( a, 10, "" );
  addPos( a, 2, "" );
  addPos( a, 8, "" );
  addPos( a, 10, "" );
  addPos( a, 1, "" );
  addPos( a, 8, "" );
  addPos( a, 10, "" );
  addPos( a, 1, "" );
  addPos( a, 10, "" );

  setDelimitorLines( a );
}

matarinfoTransDelim()
{
  a = transHuvudDelim();

  addPos( a, 10, "Kundid KLK" );
  addPos( a, 13, "Kundpostid VA" );
  addPos( a, 10, "Abonnemangsid KLK" );
  addPos( a, 8, "Abonnemangsid VA" );
  addPos( a, 3, "Mätarid VA" );
  addPos( a, 10, "Uppdragsnr KLK" );
  addPos( a, 5, "Mätarnummer" );
  addPos( a, 1, "Sifferkapacitet" );
  addPos( a, 8, "Mätarställning" );
  addPos( a, 10, "Avläsningsdatum" );
  addPos( a, 12, "Avläsingstid" );
  addPos( a, 1, "Avläsningstyp" );
  addPos( a, 8, "Beräknad mätarställning" );
  addPos( a, 10, "Beräknad datum" );
  addPos( a, 1, "Anledning beräknad mätarställning" );
  addPos( a, 7, "DYF aktuell" );
  addPos( a, 7, "DYF beslutad" );
  addPos( a, 1, "Anledning beslutad DYF" );
  addPos( a, 10, "Planerad nästa avläsning" );
  addPos( a, 10, "Planerad nästa fakturering" );
  addPos( a, 2, "Mätartranstyp" );
  addPos( a, 8, "Föregående mätarställning" );
  addPos( a, 10, "Föregående avläsningsdatum" );
  addPos( a, 1, "Föregående avläst av" );
  addPos( a, 8, "Föregående beräknad mätarställning" );
  addPos( a, 10, "Föregående beräknad datum" );
  addPos( a, 1, "Självavläsarrabatt" );

  setDelimitorLines( a );
}

extradebTransDelim()
{
  a = transHuvudDelim();

  addPos( a, 10, "" );
  addPos( a, 13, "" );
  addPos( a, 10, "" );
  addPos( a, 8, "" );
  addPos( a, 3, "" );
  addPos( a, 10, "" );
  addPos( a, 5, "" );
  addPos( a, 4, "" );
  addPos( a, 9, "" );
  addPos( a, 9, "" );
  addPos( a, 255, "" );
  addPos( a, 9, "" );

  setDelimitorLines( a );
}

showColumnInfo()
{
  for( i = 0; i < length( columnDescription ); i++ )
  {
    if( getCursorColumn() <= columnDescription[i][0] )
    {
      popupWindow( columnDescription[i][1] );
      break;
    }
  }
}

testDlg()
{
  data = "123";

  queryLineDialog( "Testdialog", "Enter a string", data );

  print( data );
}

ktest(a)
{
  switch( a )
  {
    case "abc" : print( "abc" );break;
    case "def" : print( "def" );break;
    default: print( "default" );
  }

  i = 0;

  do
  {
    if( i++ > 20 ) break;
  } while( 1 );
}

ktest2(a)
{
  switch( a )
  {
    case 5 * 2 * 2 : print( "20" ); break;
    case 6 * 2 * 2 : print( "24" ); break;
    default : print( "none" );
  }
}

aaa( &a )
{
//  for( i = 0; i < 1000; i++ )
//    a = a + [i];
  a = a + [1];
}

crash()
{
  setScreenUpdate( FALSE );
  while( lineDown() )
  {
    insertString( "a" );
    charLeft();
  }
  setScreenUpdate( TRUE );
}

toBeInStartup()
{
  bindKey( "Ctrl+l-Ctrl+k", "kundInfoDelim();" );
  bindKey( "Ctrl+l-Ctrl+a", "aboInfoDelim();" );
  bindKey( "Ctrl+l-Ctrl+m", "matarlistaDelim();" );
  bindKey( "Ctrl+l-Ctrl+i", "matarinfoDelim();" );
  bindKey( "Ctrl+l-Ctrl+l", "setDelimitorLines( [80] );" );
  bindKey( "Ctrl+l-Ctrl+t-Ctrl+k", "kundInfoTransDelim();" );
  bindKey( "Ctrl+l-Ctrl+t-Ctrl+a", "aboInfoTransDelim();" );
  bindKey( "Ctrl+l-Ctrl+t-Ctrl+m", "matarlistaTransDelim();" );
  bindKey( "Ctrl+l-Ctrl+t-Ctrl+i", "matarinfoTransDelim();" );
  bindKey( "Ctrl+l-Ctrl+t-Ctrl+e", "extradebTransDelim();" );
}
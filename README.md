# libeburc

The EB library rewritten in C++/CX favor.

Original Source: [EB Library with UTF-8 support](http://green.ribbon.to/~ikazuhiro/dic/ebu.html)

### WORKING IN PROGRESS

#### TODO
- Reading .ebz format ( Needs to port zilb )
- Auto mbstring output for book LOCALE
- Text searching
- Mixed LOCAL mapping?

### Classes
#### Main classes
`EBBook`
`EBSubbook`
`EBAppendix`
`EBAppendixSubbook`

#### Helper classes
`EBInfo`

#### Object classes
`EBPosition`
`EBHit`
`EBHook`
`EBHookSet`


#### Sample code ( C# )
```C#
string ID = "libeburc";
try
{
	EBBook ebb = await EBBook.Parse( KnownFolders.SavedPictures );

	Logger.Log( ID, "Disc Type: " + EBInfo.DiskType( ebb ), LogType.INFO );
	Logger.Log( ID, "Character Code: " + EBInfo.CharCode( ebb ), LogType.INFO );
	Logger.Log( ID, "Number of Subbooks: " + ebb.SubbookCount, LogType.INFO );

	Logger.Log( ID, "", LogType.INFO );
	foreach( EBSubbook Sbook in ebb.Subbooks )
	{
		Logger.Log( ID, "Subbook " + Sbook.Code + ":", LogType.INFO );
		await Sbook.OpenAsync();

		Logger.Log( ID, "  Title: " + Sbook.Title, LogType.INFO );
		Logger.Log( ID, "  Directory: " + Sbook.Directory, LogType.INFO );
		Logger.Log( ID, "  Search methods: " + string.Join( " ", EBInfo.SearchMethods( Sbook ) ), LogType.INFO );
		Logger.Log( ID, "  Font sizes: " + string.Join( " ", EBInfo.FontList( Sbook ) ), LogType.INFO );
		CharRange NCR = await EBInfo.GetNarrowFontRangeAsync( Sbook );
		Logger.Log( ID, string.Format( "  Narrow char range: 0x{0:X} - 0x{1:X}", NCR.Start, NCR.End ), LogType.INFO );
		CharRange WCR = await EBInfo.GetWideFontRangeAsync( Sbook );
		Logger.Log( ID, string.Format( "  Wide char range: 0x{0:X} - 0x{1:X}", WCR.Start, WCR.End ), LogType.INFO );

		// Begin Seek text test
		EBPosition Pos = Sbook.FirstPage;
		for( int i = 0; i < 2; i ++ )
		{
			Logger.Log( ID, string.Format( "  Start: {{ Page: {0}, Offset: {1} }}", Pos.Page, Pos.Offset ), LogType.INFO );
			Logger.Log( ID, "  Content: " + await Sbook.GetPageAsync( Pos ), LogType.INFO );
		}

		string[] keyword = new string[] { "水田" };
		IEnumerable<EBHit> Hits = await Sbook.SearchAysnc( keyword, EBSearchCode.EB_SEARCH_KEYWORD );
		Logger.Log( ID, string.Format( "  Search \"{0}\" hits {1} result(s)", string.Join( ", ", keyword ), Hits.Count() ) , LogType.INFO );
		foreach( EBHit Hit in Hits )
		{
			Pos = Hit.Heading;
			Logger.Log( ID, string.Format( "    Heading: {{ Page: {0}, Offset: {1} }}", Pos.Page, Pos.Offset ), LogType.INFO );
			Pos = Hit.Text;
			Logger.Log( ID, string.Format( "    Text: {{ Page: {0}, Offset: {1} }}", Pos.Page, Pos.Offset ), LogType.INFO );
			Logger.Log( ID, "    Text: " + await Sbook.GetPageAsync( Pos ), LogType.INFO );
		}
	}
}
catch( Exception ex )
{
	Logger.Log( "App", ex.Message, LogType.ERROR );

```

#### Sample code ( C++ )
```
// Pending
```

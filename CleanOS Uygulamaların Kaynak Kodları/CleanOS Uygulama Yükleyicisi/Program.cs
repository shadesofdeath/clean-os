using Spectre.Console;
using System.Text.Json;
using System.Net.Http;
using System.Diagnostics;

class Program
{
    private static readonly HttpClient client;
    private static readonly string baseUrl = "https://evergreen-api.stealthpuppy.com";

   
    private static readonly Dictionary<string, (string DisplayName, Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)> Apps)> categories = 
        new()
    {
        { "1", ("Tarayıcılar", new Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)> 
            {
                { "1", ("Microsoft Edge", "MicrosoftEdge", "--silent --system-level") },
                { "2", ("Google Chrome", "GoogleChrome", "/silent /install") },
                { "3", ("Mozilla Firefox", "MozillaFirefox", "-ms") },
                { "4", ("Opera Browser", "OperaBrowser", "/silent /allusers=1 /launchbrowser=0") },
                { "5", ("Opera GX", "OperaGXBrowser", "/silent /allusers=1 /launchbrowser=0") }
            })
        },
        { "2", ("Arşivleyiciler", new Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)>
            {
                { "1", ("7-Zip", "7Zip", "/S") },
                { "2", ("7-Zip ZS", "7ZipZS", "/S") },
                { "3", ("PeaZip", "PeaZipPeaZip", "/silent") },
                { "4", ("NanaZip", "NanaZip", "/S") }
            })
        },
        { "3", ("Medya Oynatıcılar", new Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)>
            {
                { "1", ("VLC Media Player", "VideoLanVlcPlayer", "/S") }
            })
        },
        { "4", ("PDF Araçları", new Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)>
            {
                { "1", ("Adobe Acrobat Reader DC", "AdobeAcrobatReaderDC", "/sAll") },
                { "2", ("Foxit Reader", "FoxitReader", "/silent") },
                { "3", ("Geek Software PDF24 Creator", "GeekSoftwarePDF24Creator", "/S") },
                { "4", ("Sumatra PDF Reader", "SumatraPDFReader", "/S") },
                { "5", ("Sober Lemur PDFSam Basic", "SoberLemurPDFSamBasic", "/S") },
                { "6", ("Tracker Software PDF-XChange Editor", "TrackerSoftwarePDFXChangeEditor", "/quiet") },
                { "7", ("PDF Arranger", "PDFArranger", "/S") },
                { "8", ("Foxit PDF Editor", "FoxitPDFEditor", "/silent") },
                { "9", ("pdfforge PDFCreator", "PDFForgePDFCreator", "/S") },
                { "10", ("Sober Lemur PDFSam Basic", "SoberLemurPDFSamBasic", "/S") }
            })
        },
        { "5", ("İletişim Araçları", new Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)>
            {
                { "1", ("Zoom", "Zoom", "/silent") },
                { "2", ("Microsoft Teams (classic", "MicrosoftTeamsClassic", "/silent") },
                { "3", ("Microsoft Teams (new)", "MicrosoftTeams", "/silent") },
                { "4", ("Slack", "Slack", "/silent") },
                { "5", ("Telegram Desktop", "TelegramDesktop", "/silent") },
                { "6", ("Signal for Windows", "SignalDesktop", "/silent") }

            })
        },
        { "6", ("Sistem Araçları", new Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)>
            {
                { "1", ("JAM Software TreeSize Free", "JamTreeSizeFree", "/S") },
                { "2", ("VMware Tools", "VMwareTools", "/S") },
                { "3", ("Piriform CCleaner Free", "PiriformCCleanerFree", "/S") },
                { "4", ("Microsoft PowerToys", "MicrosoftPowerToys", "/silent") },
                { "5", ("Microsoft Windows Terminal", "MicrosoftTerminal", "/silent") },
                { "6", ("Windows Auto Dark Mode", "AutoDarkMode", "/silent") },
                { "7", ("DevToys", "DevToys", "/silent") },
                { "8", ("Gsudo", "gsudo", "/silent") },
                { "9", ("Open-Shell-Menu", "OpenShellMenu", "/silent") },
            })
        },
        { "7", ("Geliştirici Araçları", new Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)>
            {
                { "1", ("Microsoft Visual Studio", "MicrosoftVisualStudio", "/silent") },
                { "2", ("Microsoft Visual Studio Code", "MicrosoftVisualStudioCode", "/silent") },
                { "3", ("VSCodium", "VSCodium", "/silent") },
                { "4", ("Git for Windows", "GitForWindows", "/silent") },
                { "5", ("Git Extensions", "GitExtensions", "/silent") },
                { "6", ("GitHub release", "GitHubRelease", "/silent") },
                { "7", ("GitHub Atom", "GitHubAtom", "/silent") },
                { "8", ("GitHub Desktop", "GitHubDesktop", "/silent") },
                { "9", ("PuTTY", "PuTTY", "/silent") },
                { "10", ("Notepad++", "NotepadPlusPlus", "/silent") },
                { "11", ("WinSCP", "WinSCP", "/silent") },
                { "12", ("Microsoft OpenJDK 11", "MicrosoftOpenJDK11", "/silent") },
                { "13", ("Microsoft OpenJDK 17", "MicrosoftOpenJDK17", "/silent") },
                { "14", ("Microsoft OpenJDK 21", "MicrosoftOpenJDK21", "/silent") },
                { "15", ("JetBrains IntelliJ IDEA", "JetBrainsIntelliJIDEA", "/silent") },
                { "16", ("Microsoft Windows Package Manager Client (Winget)", "MicrosoftWindowsPackageManagerClient", "/silent") },
                { "17", ("Oracle Java 8", "OracleJava8", "/silent") },
                { "18", ("Oracle Java 17", "OracleJava17", "/silent") },
                { "19", ("Oracle Java 20", "OracleJava20", "/silent") },
                { "20", ("Oracle Java 21", "OracleJava21", "/silent") },
                { "21", ("Oracle Java 22", "OracleJava22", "/silent") },
                { "22", ("Oracle Java 23", "OracleJava23", "/silent") },
                { "23", ("Akeo Rufus", "AkeoRufus", "/silent") },
                { "24", ("Microsoft .NET", "Microsoft.NET", "/silent") },
                { "25", ("Node.js", "NodeJs", "/silent") },
                { "26", ("Unity Editor", "UnityEditor", "/silent") },
                { "27", ("VisualCppRedistAIO", "VisualCppRedistAIO", "/silent") },
                { "28", ("WinMerge", "WinMerge", "/silent") },
                { "29", ("Wireshark", "Wireshark", "/silent") }
            })
        },
        { "8", ("Güvenlik Araçları", new Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)>
            {
                { "1", ("ESET Endpoint Antivirus", "Malwarebytes.Malwarebytes", "/silent") },
                { "2", ("KeePass Password Safe", "KeePass", "/silent") },
                { "3", ("Bitwarden Desktop", "BitwardenDesktop", "/silent") },
                { "4", ("VeraCrypt", "VeraCrypt", "/silent") },
                { "5", ("Proton VPN", "ProtonVPN", "/silent") },
                { "6", ("Tor Browser", "TorBrowser", "/silent") },
                { "7", ("Wireshark", "Wireshark", "/silent") },
                { "8", ("PuTTY", "PuTTY", "/silent") },
                { "9", ("KeePassXC", "KeePassXC", "/silent") }
            })
        },
        { "9", ("Ses ve Video Araçları", new Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)>
            {
                { "1", ("Audacity", "Audacity", "/silent") },
                { "2", ("OBS Studio", "OBSStudio", "/silent") },
                { "3", ("OBS Studio Alt", "OBSStudioAlt", "/silent") },
                { "4", ("HandBrake", "Handbrake", "/silent") }
            })
        },
        { "10", ("Alıntı ve Düzenleme Araçları", new Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)>
        {
            { "1", ("Flameshot", "Flameshot", "/silent") },
            { "2", ("GIMP", "GIMP", "/silent") },
            { "3", ("Greenshot", "Greenshot", "/silent") },
            { "4", ("ImageGlass", "ImageGlass", "/silent") },
            { "5", ("ImageMagick", "ImageMagick", "/silent") }
        })
    },

    };

    static Program()
    {
        client = new HttpClient();
        client.DefaultRequestHeaders.Add("User-Agent", "CleanOS AppInstaller");
        client.Timeout = TimeSpan.FromSeconds(30);
        Console.Title = "CleanOS App Installer (Beta)";
    }

    private static string ReadInput()
    {
        string input = "";
        while (true)
        {
            var key = Console.ReadKey(true);
            
            if (key.Key == ConsoleKey.Enter)
                break;
            
            if (key.Key == ConsoleKey.Backspace && input.Length > 0)
            {
                input = input[..^1];
                Console.Write("\b \b"); 
                continue;
            }
            
            if (char.IsDigit(key.KeyChar))
            {
                input += key.KeyChar;
                Console.Write(key.KeyChar);
            }
        }
        Console.WriteLine(); 
        return input;
    }

    static async Task Main(string[] args)
    {
        while (true)
        {
            Console.Clear();
            AnsiConsole.Write(new FigletText("CleanOS").Color(Color.DeepSkyBlue3));
            AnsiConsole.WriteLine("CleanOS App Installer (Beta)\n");

            var categorySelection = AnsiConsole.Prompt(
                new SelectionPrompt<string>()
                    .Title("[lightcoral]Kategori seçin:[/]")
                    .PageSize(10)
                    .HighlightStyle(new Style(foreground: Color.DeepSkyBlue3))
                    .AddChoices(categories.Select(cat => $"{cat.Key}. {cat.Value.DisplayName}")
                        .Concat(new[] { "0. Çıkış" })));

            var categoryKey = categorySelection.Split('.')[0];

            if (categoryKey == "0")
            {
                AnsiConsole.MarkupLine("\n[lightcoral]Çıkmak istediğinize emin misiniz? (E/H):[/]");
                var confirm = Console.ReadKey(true);
                if (confirm.Key == ConsoleKey.E)
                    break;
                continue;
            }

            if (categories.TryGetValue(categoryKey, out var categoryInfo))
            {
                await ShowCategoryApps(categoryInfo.DisplayName, categoryInfo.Apps);
            }
            else
            {
                AnsiConsole.MarkupLine("[lightcoral]Geçersiz kategori![/]");
                await Task.Delay(2000);
            }
        }
    }

    static async Task ShowCategoryApps(string categoryName, Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)> apps)
    {
        while (true)
        {
            Console.Clear();
            AnsiConsole.Write(new FigletText(categoryName).Color(Color.DeepSkyBlue3));

            var selection = AnsiConsole.Prompt(
                new SelectionPrompt<string>()
                    .Title("[lightcoral]Uygulama seçin (ESC ile geri dönün):[/]")
                    .PageSize(10)
                    .HighlightStyle(new Style(foreground: Color.DeepSkyBlue3))
                    .AddChoices(apps.Select(app => $"{app.Key}. {app.Value.DisplayName}")
                        .Concat(new[] { "0. Geri" })));

            if (selection.StartsWith("0"))
                break;

            var key = selection.Split('.')[0];
            if (apps.TryGetValue(key, out var appInfo))
            {
                await GetBrowserInfo(appInfo.ApiName, appInfo.DisplayName, apps, key);
                break;
            }
        }
    }

    static async Task TestApiConnection()
    {
        try
        {
            var response = await client.GetAsync($"{baseUrl}/apps");
            response.EnsureSuccessStatusCode();
        }
        catch (Exception)
        {
            throw new Exception("API bağlantısı kurulamadı. İnternet bağlantınızı kontrol edin.");
        }
    }

    static async Task GetBrowserInfo(string browserName, string displayName, Dictionary<string, (string DisplayName, string ApiName, string SilentArgs)> apps, string selection)
    {
        try
        {
            Console.Clear();
            var response = await client.GetAsync($"{baseUrl}/app/{browserName}");
            response.EnsureSuccessStatusCode();
            
            var jsonString = await response.Content.ReadAsStringAsync();
            var jsonDoc = JsonSerializer.Deserialize<JsonElement>(jsonString);

            JsonElement browserInfo;
            if (jsonDoc.ValueKind == JsonValueKind.Object && jsonDoc.TryGetProperty("value", out var valueArray))
            {
                browserInfo = valueArray;
            }
            else
            {
                browserInfo = jsonDoc;
            }

            if (browserInfo.ValueKind != JsonValueKind.Array || browserInfo.GetArrayLength() == 0)
            {
                throw new Exception($"{browserName} için bilgi bulunamadı.");
            }

            var filteredBrowserInfo = browserInfo.EnumerateArray()
                .Where(browser => 
                {
                    bool isValid = true;

                    if (browser.TryGetProperty("Type", out var typeProp))
                    {
                        var type = typeProp.ToString().ToLower();
                        isValid = type == "exe" || type == "msi" || type == "msixbundle" || type == "msix" || type == "zip";
                    }

                    if (isValid && browser.TryGetProperty("Architecture", out var archProp))
                    {
                        var arch = archProp.ToString().ToLower();
                        isValid = (arch == "x64" || arch == "x86");
                    }

                    if (isValid && browser.TryGetProperty("Language", out var languageProp))
                    {
                        var language = languageProp.ToString().ToLower();
                        isValid = IsValidLanguage(language);
                    }

                    return isValid;
                })
                .ToList();

            if (!filteredBrowserInfo.Any())
            {
                throw new Exception($"{displayName} için uygun sürüm bulunamadı (x86/x64, .exe/.msi).");
            }

            var allProperties = new HashSet<string>();
            foreach (var browser in filteredBrowserInfo)
            {
                foreach (var prop in browser.EnumerateObject())
                {
                    allProperties.Add(prop.Name);
                }
            }

            var excludedProperties = new HashSet<string> { 
                "Hash", 
                "URI", 
                "MD5",
                "Md5",
                "Support",
                "Sha512",
                "Sha256",
                "Sha",
                "Platform",
                "Filename",
                "Checksum"
            };

            bool hasLanguageProperty = filteredBrowserInfo.Any(b => b.TryGetProperty("Language", out _));
            if (!hasLanguageProperty)
            {
                excludedProperties.Add("Language");
            }

            allProperties.RemoveWhere(prop => excludedProperties.Contains(prop));

            var table = new Table()
                .Border(TableBorder.Horizontal)
                .BorderColor(Color.DodgerBlue1)
                .Title($"[dodgerblue1]{displayName}[/] Versiyonları")
                .Centered()
                .Expand();
            
            Console.Clear();
            Console.WriteLine("\n");
            table.Title = new TableTitle($"[dodgerblue1]{displayName}[/] Versiyonları");
            
            table.AddColumn(new TableColumn("[grey100]#[/]").Width(3));
            
            foreach (var prop in allProperties)
            {
                var columnTitle = GetFriendlyColumnName(prop);
                table.AddColumn(new TableColumn($"[grey100]{columnTitle}[/]").Width(15));
            }

            int rowNumber = 1;
            foreach (var browser in filteredBrowserInfo)
            {
                var rowValues = new List<string> { $"[dodgerblue1]{rowNumber}[/]" };
                
                foreach (var header in allProperties)
                {
                    var propValue = browser.TryGetProperty(header, out var value) 
                        ? FormatValue(value.ToString(), header)
                        : "-";
                    rowValues.Add(propValue);
                }
                
                table.AddRow(rowValues.ToArray());
                if (rowNumber < filteredBrowserInfo.Count())
                {
                    table.AddRow(new Rule("").RuleStyle(Style.Parse("dim")));
                }
                rowNumber++;
            }

            AnsiConsole.Write(table);
            AnsiConsole.WriteLine();
            AnsiConsole.MarkupLine("[lightcoral]İndirmek istediğiniz uygulamanın numarasını girin (Ana menü için 0):[/]");
            var choice = ReadInput();

            if (choice == "0")
                return;

            AnsiConsole.MarkupLine($"[gold3_1]Seçilen sürüm: {choice}[/]");

            if (int.TryParse(choice, out int selectedIndex) && selectedIndex > 0 && selectedIndex <= filteredBrowserInfo.Count())
            {
                var selectedBrowser = filteredBrowserInfo.ElementAt(selectedIndex - 1);
                if (selectedBrowser.TryGetProperty("URI", out var uriProp))
                {
                    var downloadUrl = uriProp.GetString();
                    if (string.IsNullOrEmpty(downloadUrl))
                    {
                        throw new Exception("İndirme bağlantısı bulunamadı.");
                    }
                    var fileName = Path.GetFileName(new Uri(downloadUrl!).LocalPath);
                    var downloadPath = Path.Combine(Path.GetTempPath(), fileName);

                    await AnsiConsole.Progress()
                        .StartAsync(async ctx =>
                        {
                            var downloadTask = ctx.AddTask("[steelblue1]İndiriliyor[/]");
                            
                            using (var response = await client.GetAsync(downloadUrl, HttpCompletionOption.ResponseHeadersRead))
                            using (var fileStream = new FileStream(downloadPath, FileMode.Create, FileAccess.Write, FileShare.None))
                            {
                                var totalBytes = response.Content.Headers.ContentLength ?? -1L;
                                var buffer = new byte[8192];
                                var bytesRead = 0L;
                                
                                using (var stream = await response.Content.ReadAsStreamAsync())
                                {
                                    while (true)
                                    {
                                        var read = await stream.ReadAsync(buffer);
                                        if (read == 0) break;
                                        
                                        await fileStream.WriteAsync(buffer.AsMemory(0, read));
                                        bytesRead += read;
                                        
                                        if (totalBytes != -1)
                                            downloadTask.Value = (double)bytesRead / totalBytes * 100;
                                    }
                                }
                            }
                        });

                    if (File.Exists(downloadPath))
                    {
                        var fileExtension = Path.GetExtension(downloadPath).ToLower();
                        var silentArgs = apps[selection].SilentArgs;

                        if (fileExtension is ".zip" or ".rar" or ".7z")
                        {
                            var desktopPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Desktop), fileName);
                            File.Copy(downloadPath, desktopPath, true);
                            AnsiConsole.MarkupLine("[yellow]Arşiv dosyası tespit edildi![/]");
                            AnsiConsole.MarkupLine($"[green]Dosya masaüstüne kaydedildi: {fileName}[/]");
                            AnsiConsole.MarkupLine("[grey100]Not: Arşiv dosyaları için otomatik kurulum desteklenmemektedir.[/]");
                        }
                        else
                        {
                            var installInfo = new ProcessStartInfo
                            {
                                FileName = fileExtension switch
                                {
                                    ".msi" => "msiexec.exe",
                                    ".msix" or ".msixbundle" => "powershell.exe",
                                    _ => downloadPath 
                                },
                                UseShellExecute = true,
                                Verb = "runas"
                            };

                            switch (fileExtension)
                            {
                                case ".msi":
                                    installInfo.Arguments = $"/i \"{downloadPath}\" /quiet /passive /norestart";
                                    break;
                                case ".msix":
                                case ".msixbundle":
                                    installInfo.Arguments = $"-ExecutionPolicy Bypass -Command \"Add-AppxPackage -Path '{downloadPath}'\"";
                                    break;
                                default: // .exe
                                    installInfo.Arguments = silentArgs;
                                    installInfo.FileName = downloadPath;
                                    break;
                            }

                            AnsiConsole.MarkupLine($"[yellow]Kurulum başlatılıyor...[/]");
                            try
                            {
                                using var process = Process.Start(installInfo);
                                if (process != null)
                                {
                                    await process.WaitForExitAsync();
                                    if (process.ExitCode == 0)
                                        AnsiConsole.MarkupLine("[chartreuse2]Kurulum başarıyla tamamlandı![/]");
                                    else
                                        AnsiConsole.MarkupLine($"[red]Kurulum hatası! Çıkış kodu: {process.ExitCode}[/]");
                                }
                            }
                            catch (Exception ex)
                            {
                                AnsiConsole.MarkupLine($"[red]Kurulum hatası: {ex.Message}[/]");
                            }
                        }

                        if (!(fileExtension is string && (fileExtension == ".zip" || fileExtension == ".rar" || fileExtension == ".7z")))
                        {
                            try { File.Delete(downloadPath); }
                            catch { /* Temizleme hatalarını yoksay */ }
                        }
                    }
                }
            }

            AnsiConsole.MarkupLine("\n[grey100]Devam etmek için bir tuşa basın...[/]");
            Console.ReadKey(true);
        }
        catch (JsonException ex)
        {
            throw new Exception($"API yanıtı işlenirken hata oluştu: {ex.Message}");
        }
        catch (Exception ex)
        {
            throw new Exception($"Beklenmeyen hata: {ex.Message}");
        }
    }

    private static bool IsValidLanguage(string language)
    {
        if (string.IsNullOrEmpty(language)) return false;
        
        var validLanguages = new[] 
        { 
            "tr-tr",
            "tr",
            "turkish",
            "english",
            "all"
        };

        return validLanguages.Any(validLang => 
            language.Equals(validLang, StringComparison.OrdinalIgnoreCase) ||
            language.Replace(" ", "").Equals(validLang, StringComparison.OrdinalIgnoreCase)
        );
    }

    private static string GetFriendlyColumnName(string propertyName)
    {
        return propertyName switch
        {
            "Version" => "Sürüm",
            "Channel" => "Kanal",
            "Release" => "Yayın",
            "Architecture" => "Mimari",
            "Type" => "Tip",
            "Date" => "Tarih",
            "URI" => "İndirme Linki",
            "Hash" => "Hash",
            "Filename" => "Dosya Adı",
            "InstallerType" => "Kurulum Tipi",
            "Language" => "Dil",
            "Size" => "Boyut",
            "Build" => "Yapı",
            "Edition" => "Sürüm",
            "FullVersion" => "Tam Sürüm",
            "Installer" => "Kurulum Dosyası",
            "InstallerArgs" => "Kurulum Argümanları",
            "InstallerSilentArgs" => "Sessiz Kurulum Argümanları",
            "InstallerExitCodes" => "Kurulum Çıkış Kodları",
            "InstallerSuccessExitCodes" => "Başarılı Kurulum Çıkış Kodları",
            "InstallerSuccessRegex" => "Başarılı Kurulum Regex",
            "LTS" => "Uzun Süreli Destek",
            "StartDate" => "Başlangıç Tarihi",
            "Revision" => "Revizyon",


            _ => propertyName
        };
    }

    private static string FormatValue(string value, string propertyName)
    {
        if (string.IsNullOrEmpty(value)) return "-";

        if (propertyName == "Size" && long.TryParse(value, out long byteSize))
        {
            double mbSize = Math.Round(byteSize / 1024.0 / 1024.0, 2);
            return $"[blue]{mbSize} MB[/]";
        }
        
        var lowerValue = value.ToLower().Trim();
        
        return lowerValue switch
        {
            var v when v.Contains("tr-tr", StringComparison.OrdinalIgnoreCase) || v.Contains("turkish", StringComparison.OrdinalIgnoreCase) => $"[green1]{value}[/]",
            var v when v.Contains("tr", StringComparison.OrdinalIgnoreCase) => $"[skyblue1]{value}[/]",
            var v when v.Contains("x64", StringComparison.OrdinalIgnoreCase) => $"[greenyellow]{value}[/]",
            var v when v.Contains("x86", StringComparison.OrdinalIgnoreCase) => $"[yellow3_1]{value}[/]",
            var v when v.Contains("stable", StringComparison.OrdinalIgnoreCase) => $"[green1]{value}[/]",
            var v when v.Contains("support", StringComparison.OrdinalIgnoreCase) => $"[lime]{value}[/]",
            var v when v.Contains("current", StringComparison.OrdinalIgnoreCase) => $"[darkolivegreen1]{value}[/]",
            var v when v.Contains("edge", StringComparison.OrdinalIgnoreCase) => $"[skyblue2]{value}[/]",
            var v when v.Contains("insider", StringComparison.OrdinalIgnoreCase) || 
                      v.Contains("dev", StringComparison.OrdinalIgnoreCase) || 
                      v.Contains("canary", StringComparison.OrdinalIgnoreCase) || 
                      v.Contains("developer", StringComparison.OrdinalIgnoreCase) => $"[orange1]{value}[/]",
            var v when v.Contains("beta", StringComparison.OrdinalIgnoreCase) => $"[gold1]{value}[/]",
            var v when v.Contains("consumer", StringComparison.OrdinalIgnoreCase) => $"[mediumpurple]{value}[/]",
            var v when v.Contains("enterprise", StringComparison.OrdinalIgnoreCase) => $"[skyblue1]{value}[/]",
            _ => $"[grey100]{value}[/]"
        };
    }

    private static IEnumerable<string> ChunkString(string str, int chunkSize)
    {
        for (int i = 0; i < str.Length; i += chunkSize)
        {
            yield return str.Substring(i, Math.Min(chunkSize, str.Length - i));
        }
    }
}

using System;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.Storage.Search;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace IoTCoreMediaPlayer
{
    public sealed partial class MainPage : Page
    {

        private QueryOptions queryOptions;

        // from https://msdn.microsoft.com/en-us/library/windows/apps/xaml/mt188703.aspx?f=255&MSPPError=-2147217396
        private string[] mediaFileExtensions = {
            // music
            ".qcp",
            ".wav",
            ".mp3",
            ".m4r",
            ".m4a",
            ".aac",
            ".amr",
            ".wma",
            ".3g2",
            ".3gp",
            ".mp4",
            ".wm",
            ".asf",
            ".3gpp",
            ".3gp2",
            ".mpa",
            ".adt",
            ".adts",
            ".pya",

            // video
            ".wm",
            ".m4v",
            ".wmv",
            ".asf",
            ".mov",
            ".mp4",
            ".3g2",
            ".3gp",
            ".mp4v",
            ".avi",
            ".pyv",
            ".3gpp",
            ".3gp2"
        };

        //const string NetworkFolder = ">Network (NYI)";

        StorageFolder currentFolder;
        StorageFile Picker_SelectedFile;

        public MainPage()
        {
            this.InitializeComponent();

            mediaElement.AutoPlay = false;
            mediaElement.MediaFailed += MediaElement_MediaFailed;

            queryOptions = new QueryOptions(CommonFileQuery.OrderByName, mediaFileExtensions);
            queryOptions.FolderDepth = FolderDepth.Shallow;
        }

        private void SetMainPageControlEnableState(bool isEnabled)
        {
            btnBrowse.IsEnabled = isEnabled;
            btnClear.IsEnabled = isEnabled;
            btnOpen.IsEnabled = isEnabled;
            txtFileName.IsEnabled = isEnabled;
            mediaElement.TransportControls.IsEnabled = isEnabled;
        }

        private async void Picker_Show()
        {
            SetMainPageControlEnableState(false);
            await Picker_Populate();
            grdPicker.Visibility = Visibility.Visible;
        }

        private void Picker_Hide()
        {
            SetMainPageControlEnableState(true);
            grdPicker.Visibility = Visibility.Collapsed;
        }

        private async Task Picker_Populate()
        {
            Picker_SelectedFile = null;
            if (currentFolder == null)
            {
                lstFiles.Items.Clear();
                lstFiles.Items.Add(">Documents");
                lstFiles.Items.Add(">Music");
                lstFiles.Items.Add(">Videos");
                lstFiles.Items.Add(">RemovableStorage");
                //lstFiles.Items.Add(NetworkFolder);
            }
            else
            {
                lstFiles.Items.Clear();
                lstFiles.Items.Add(">..");
                var folders = await currentFolder.GetFoldersAsync();
                foreach (var f in folders)
                {
                    lstFiles.Items.Add(">" + f.Name);
                }
                var query = currentFolder.CreateFileQueryWithOptions(queryOptions);
                var files = await query.GetFilesAsync();
                foreach (var f in files)
                {
                    lstFiles.Items.Add(f.Name);
                }
            }
        }

        private async Task<bool> Picker_BrowseTo(string filename)
        {
            Picker_SelectedFile = null;
            if (currentFolder == null)
            {
                switch (filename)
                {
                    case ">Documents":
                        currentFolder = KnownFolders.DocumentsLibrary;
                        break;
                    case ">Music":
                        currentFolder = KnownFolders.MusicLibrary;
                        break;
                    case ">Videos":
                        currentFolder = KnownFolders.VideosLibrary;
                        break;
                    case ">RemovableStorage":
                        currentFolder = KnownFolders.RemovableDevices;
                        break;
                    //case NetworkFolder:
                    //    // special case... NYI
                    //    return false;
                    default:
                        throw new Exception("unexpected");
                }
                lblBreadcrumb.Text = "> " + filename.Substring(1);
            }
            else
            {
                if (filename == ">..")
                {
                    await Picker_FolderUp();
                }
                else if (filename[0] == '>')
                {
                    var foldername = filename.Substring(1);
                    var folder = await currentFolder.GetFolderAsync(foldername);
                    currentFolder = folder;
                    lblBreadcrumb.Text += " > " + foldername;
                }
                else
                {
                    Picker_SelectedFile = await currentFolder.GetFileAsync(filename);
                    return true;
                }
            }
            await Picker_Populate();
            return false;
        }

        async Task Picker_FolderUp()
        {
            if (currentFolder == null)
            {
                return;
            }
            try
            {
                var folder = await currentFolder.GetParentAsync();
                currentFolder = folder;
                if (currentFolder == null)
                {
                    lblBreadcrumb.Text = ">";
                }
                else
                {
                    var breadcrumb = lblBreadcrumb.Text;
                    breadcrumb = breadcrumb.Substring(0, breadcrumb.LastIndexOf('>') - 1);
                    lblBreadcrumb.Text = breadcrumb;
                }
            }
            catch (Exception)
            {
                currentFolder = null;
                lblBreadcrumb.Text = ">";
            }
        }

        async void SelectFile()
        {
            Picker_Hide();
            try
            {
                if (Picker_SelectedFile != null)
                {
                    txtFileName.Text = Picker_SelectedFile.Path;
                    var stream = await Picker_SelectedFile.OpenAsync(Windows.Storage.FileAccessMode.Read);
                    mediaElement.SetSource(stream, Picker_SelectedFile.ContentType);
                    mediaElement.TransportControls.Focus(FocusState.Programmatic);
                }
            }
            catch (Exception ex)
            {
                lblError.Text = ex.Message;
                lblError.Visibility = Visibility.Visible;
            }
        }

        private void MediaElement_MediaFailed(object sender, ExceptionRoutedEventArgs e)
        {
            lblError.Text = e.ErrorMessage;
            lblError.Visibility = Visibility.Visible;
        }

        private void btnBrowse_Click(object sender, RoutedEventArgs e)
        {
            lblError.Visibility = Visibility.Collapsed;
            Picker_Show();
        }

        private async void btnOpen_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                lblError.Visibility = Visibility.Collapsed;
                var file = await StorageFile.GetFileFromPathAsync(txtFileName.Text);
                var stream = await file.OpenAsync(Windows.Storage.FileAccessMode.Read);
                mediaElement.SetSource(stream, file.ContentType);
            }
            catch (Exception ex)
            {
                lblError.Text = ex.Message;
                lblError.Visibility = Visibility.Visible;
            }
        }

        private void btnClear_Click(object sender, RoutedEventArgs e)
        {
            lblError.Visibility = Visibility.Collapsed;
            txtFileName.Text = "";
            mediaElement.Source = null;
        }

        private void txtFileName_TextChanged(object sender, TextChangedEventArgs e)
        {
            lblError.Visibility = Visibility.Collapsed;
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            Picker_Hide();
        }

        private async void lstFiles_KeyUp(object sender, KeyRoutedEventArgs e)
        {
            if (lstFiles.SelectedItem != null && e.Key == Windows.System.VirtualKey.Enter)
            {
                if (await Picker_BrowseTo(lstFiles.SelectedItem.ToString()))
                {
                    SelectFile();
                }
                else
                {
                    lstFiles.Focus(FocusState.Keyboard);
                }
            }
        }

        private async void lstFiles_DoubleTapped(object sender, DoubleTappedRoutedEventArgs e)
        {
            if (lstFiles.SelectedItem != null)
            {
                if (await Picker_BrowseTo(lstFiles.SelectedItem.ToString()))
                {
                    SelectFile();
                }
                else
                {
                    lstFiles.Focus(FocusState.Keyboard);
                }
            }
        }

        private async void btnSelect_Click(object sender, RoutedEventArgs e)
        {
            if (lstFiles.SelectedItem != null)
            {
                if (await Picker_BrowseTo(lstFiles.SelectedItem.ToString()))
                {
                    SelectFile();
                }
                else
                {
                    lstFiles.Focus(FocusState.Keyboard);
                }
            }
        }
    }
}

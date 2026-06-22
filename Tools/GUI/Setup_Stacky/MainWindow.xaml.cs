using System.IO; // For System.IO.Path and System.IO.File
using System.Windows;
// Remove 'using System.Windows.Forms;' to stop global ambiguity

namespace Setup_Stacky
{
    public partial class MainWindow : Window
    {
        public MainWindow() => InitializeComponent();

        private void BrowseStackDir_Click(object sender, RoutedEventArgs e)
        {
            // Use fully qualified name for WinForms dialogs
            using (var dialog = new System.Windows.Forms.FolderBrowserDialog())
            {
                // Explicitly check against WinForms DialogResult
                if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                    txtStackDir.Text = dialog.SelectedPath;
            }
        }

        private void BrowseExe_Click(object sender, RoutedEventArgs e)
        {
            // Use WPF's OpenFileDialog (avoids WinForms conflict)
            var dialog = new Microsoft.Win32.OpenFileDialog { Filter = "Executables (*.exe)|*.exe" };
            if (dialog.ShowDialog() == true)
                txtStackyExe.Text = dialog.FileName;
        }

        private void btnCreate_Click(object sender, RoutedEventArgs e)
		{
			try
			{
				// 1. Gather Inputs
				string stackDir = txtStackDir.Text;
				string stackyExe = txtStackyExe.Text;
				string shortcutName = string.IsNullOrWhiteSpace(txtShortcutName.Text)
									? "Stacky_" + System.IO.Path.GetFileName(stackDir)
									: txtShortcutName.Text;
		
				// 2. Validation
				if (string.IsNullOrWhiteSpace(stackDir) || string.IsNullOrWhiteSpace(stackyExe))
					throw new Exception("Please provide both a Stack Directory and the stacky.exe path.");
		
				if (!Directory.Exists(stackDir)) Directory.CreateDirectory(stackDir);
				if (!System.IO.File.Exists(stackyExe)) throw new FileNotFoundException("Executable not found.");
		
				// 3. Create Config
				string configPath = System.IO.Path.Combine(stackDir, "stacky.json");
				if (!System.IO.File.Exists(configPath) || chkForce.IsChecked == true)
				{
					string defaultJson = "{\n    \"theme\": \"auto\",\n    \"overrides\": {}\n}";
					System.IO.File.WriteAllText(configPath, defaultJson);
				}
		
				// 4. Create Shortcut (Late-Bound for .NET Core/Modern .NET Compatibility)
				string desktop = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
				string shortcutPath = System.IO.Path.Combine(desktop, shortcutName + ".lnk");

                // Dynamically instantiate WScript.Shell
                Type? wshType = Type.GetTypeFromProgID("WScript.Shell");
                if (wshType != null)
                {
                    dynamic? shell = Activator.CreateInstance(wshType);
                    if (shell != null)
                    {
                        dynamic link = shell.CreateShortcut(shortcutPath);
                        link.TargetPath = stackyExe;
                        link.Arguments = $"\"{stackDir}\"";
                        link.WorkingDirectory = System.IO.Path.GetDirectoryName(stackyExe);
                        link.IconLocation = stackyExe;
                        link.Save();
                    }
                }
                else
                {
                    throw new Exception("Could not access Windows Script Host.");
                }

                // 5. Notify
                System.Windows.MessageBox.Show("Success: Stack and shortcut created!", "Setup Complete", MessageBoxButton.OK, MessageBoxImage.Information);
			}
			catch (Exception ex)
			{
				System.Windows.MessageBox.Show($"Error: {ex.Message}", "Failed", MessageBoxButton.OK, MessageBoxImage.Error);
			}
		}
    }
}
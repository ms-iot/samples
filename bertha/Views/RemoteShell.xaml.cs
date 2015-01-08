// © Copyright(C) Microsoft. All rights reserved.

using SharpDX;
using SharpDX.Direct3D11;
using SharpDX.Toolkit.Graphics;
using System;
using System.Diagnostics;
using Windows.Foundation;
using Windows.Graphics.Display;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using D2D = SharpDX.Direct2D1;
using DW = SharpDX.DirectWrite;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace bertha
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class RemoteShell : Page
    {

        // set appropiate debug level based on build type
        private const D2D.DebugLevel D2DDebugLevel =
         //        #if DEBUG
         D2D.DebugLevel.Information
        //        #else
        //         D2D.DebugLevel.Error
        //        #endif
        ;

        // will hold all disposable resources to dispose them all at once
        private readonly DisposeCollector _disposeCollector = new DisposeCollector();

        // indicates if the frame needs to be redrawn
        private bool _isDirty;

        private GraphicsDevice _graphicsDevice; // encapsulates Direct3D11 Device and DeviceContext
        private GraphicsPresenter _presenter; // encapsulates the SwapChain, Backbuffer and DepthStencil buffer

        // ReSharper disable InconsistentNaming
        private D2D.Device _d2dDevice;
        private D2D.DeviceContext _d2dDeviceContext;
        // ReSharper restore InconsistentNaming
        private DW.Factory1 _dwFactory;

        // main render target (the backbuffer) - needs to be disposed before resizing the SwapChain otherwise the resize call will fail.
        private D2D.Bitmap1 _bitmapTarget;

        // scene-specific content
        private D2D.Brush _textBrush;
        private DW.TextLayout _textLayout;
        private DW.TextFormat _textFormat;

        private Console m_console;
        private bool bTelnetConnected = false;
        private TelnetClient telnetClient;

        private bool bIsControl = false;
        private bool bIsShifted = false;

        public RemoteShell()
        {
            this.InitializeComponent();
            swapChainPanel.Loaded += HandleLoaded;
            swapChainPanel.Unloaded += HandleUnloaded;
            swapChainPanel.SizeChanged += HandleSizeChanged;

            Window.Current.CoreWindow.KeyDown += CoreWindow_KeyDown;
            Window.Current.CoreWindow.KeyUp += CoreWindow_KeyUp;

            m_console = new Console();
            m_console.Write(sConnectString);

            telnetClient = new TelnetClient();

            DispatcherTimer dt = new DispatcherTimer();
            dt.Interval = new TimeSpan(0, 0, 0, 0, 16);
            dt.Tick += dt_Tick;
            dt.Start();
        }

        string sConnectString = "Connect to: ";

        void dt_Tick(object sender, object e)
        {
            _isDirty = true;
            m_console.Tick();
            PerformRendering();
        }

        /// <summary>
        /// Disposes all D3D and D2D resources and unloads the content.
        /// </summary>
        public void Dispose()
        {
            DisposeAndUnloadContent();
        }

        /// <summary>
        /// Marks the surface to be redrawn at the next <see cref="CompositionTarget.Rendering"/> event.
        /// </summary>
        public void Redraw()
        {
            _isDirty = true;
        }

        private void CreateDeviceAndLoadContent()
        {
            Debug.Assert(_graphicsDevice == null);

            var flags = GetDeviceCreationFlags();
            Debug.Assert(flags.HasFlag(DeviceCreationFlags.BgraSupport)); // mandatory for D2D support

            // initialize Direct3D11 - this is the bridge between Direct2D and control surface
            _graphicsDevice = _disposeCollector.Collect(GraphicsDevice.New(flags));

            // get the low-level DXGI device reference
            using (var dxgiDevice = ((Device)_graphicsDevice).QueryInterface<SharpDX.DXGI.Device>())
            {
                // create D2D device sharing same GPU driver instance
                _d2dDevice = _disposeCollector.Collect(new D2D.Device(dxgiDevice, new D2D.CreationProperties { DebugLevel = D2DDebugLevel }));

                // create D2D device context used in drawing and resource creation
                // this allows us to not recreate the resources if render target gets recreated because of size change
                _d2dDeviceContext = _disposeCollector.Collect(new D2D.DeviceContext(_d2dDevice, D2D.DeviceContextOptions.EnableMultithreadedOptimizations));
            }

            // device-independent factory used to create all DirectWrite resources
            _dwFactory = _disposeCollector.Collect(new SharpDX.DirectWrite.Factory1());

            // load scene-specific content:
            LoadContent();
        }

        /// <summary>
        /// Helper method that provides device creation flags.
        /// </summary>
        /// <returns></returns>
        private DeviceCreationFlags GetDeviceCreationFlags()
        {
            return DeviceCreationFlags.BgraSupport;
            //#if DEBUG
            // | DeviceCreationFlags.Debug
            //#endif
            //;
        }

        /// <summary>
        /// Unloads content and disposes all unmanaged resources.
        /// </summary>
        private void DisposeAndUnloadContent()
        {
            _disposeCollector.DisposeAndClear();

            _d2dDeviceContext = null;
            _d2dDevice = null;
            _dwFactory = null;
            _presenter = null;
            _graphicsDevice = null;
        }

        /// <summary>
        /// Prepares control surface for rendering.
        /// </summary>
        private void StartRendering()
        {
            Debug.Assert(_presenter == null);
            Debug.Assert(_graphicsDevice.Presenter == null);
            Debug.Assert(_graphicsDevice != null);

            Redraw();

            var parameters = new PresentationParameters((int)ActualWidth, (int)ActualHeight, swapChainPanel);

            _presenter = _disposeCollector.Collect(new SwapChainGraphicsPresenter(_graphicsDevice, parameters));
            _graphicsDevice.Presenter = _presenter;

            Debug.Assert(_bitmapTarget == null);

            CreateD2DRenderTarget();

            CompositionTarget.Rendering += HandleRendering;
            // TEMP (Alecont)
            ResetSize(swapChainPanel.RenderSize);
        }

        /// <summary>
        /// Creates the D2D render target from the associated swap chain.
        /// </summary>
        private void CreateD2DRenderTarget()
        {
            var renderTarget = _presenter.BackBuffer;

            var dpi = DisplayProperties.LogicalDpi;

            // 1. Use same format as the underlying render target with premultiplied alpha mode
            // 2. Use correct DPI
            // 3. Deny drawing direct calls and specify that this is a render target.
            var bitmapProperties = new D2D.BitmapProperties1(new SharpDX.Direct2D1.PixelFormat(renderTarget.Format, D2D.AlphaMode.Premultiplied),
                                                             dpi,
                                                             dpi,
                                                             D2D.BitmapOptions.CannotDraw | D2D.BitmapOptions.Target);

            // create the bitmap render target and assign it to the device context
            _bitmapTarget = _disposeCollector.Collect(new D2D.Bitmap1(_d2dDeviceContext, renderTarget, bitmapProperties));
            _d2dDeviceContext.Target = _bitmapTarget;
        }

        /// <summary>
        /// Stops rendering on control surface.
        /// </summary>
        private void StopRendering()
        {
            Debug.Assert(_presenter != null);
            Debug.Assert(_graphicsDevice.Presenter != null);

            CompositionTarget.Rendering -= HandleRendering;

            DisposeD2DRenderTarget();

            _graphicsDevice.Presenter = null;
            _disposeCollector.RemoveAndDispose(ref _presenter);
        }

        /// <summary>
        /// Removes and disposes the D2D bitmap render target to allow SwapChain resize.
        /// </summary>
        private void DisposeD2DRenderTarget()
        {
            _d2dDeviceContext.Target = null;
            _disposeCollector.RemoveAndDispose(ref _bitmapTarget);
        }

        /// <summary>
        /// Resets the size of the surface-dependent resources (swapchain and D2D render target)
        /// </summary>
        /// <param name="size"></param>
        private void ResetSize(Size size)
        {
            if (_presenter == null) return;

            if (m_console != null)
                m_console.SetSize(size);

            Redraw();

            DisposeD2DRenderTarget();
            _presenter.Resize((int)size.Width, (int)size.Height, _presenter.Description.BackBufferFormat);
            CreateD2DRenderTarget();
        }

        /// <summary>
        /// Checks if surface needs to be redrawn and performs the rendering.
        /// </summary>
        private void PerformRendering()
        {
            Debug.Assert(_graphicsDevice.Presenter != null);

            if (!_isDirty) return;
            _isDirty = false;

            //            if (!m_console.bDirty)
            //                return;
            //            m_console.bDirty = false;

            if (!BeginDrawFrame())
            {
                Redraw();
                return;
            }

            ClearRenderTarget();

            DrawContent();

            EndDrawFrame();
        }

        /// <summary>
        /// Checks the device state and prepares it for frame rendering.
        /// </summary>
        /// <returns><c>true</c> - if device is ready for drawing, <c>false</c> - otherwise.</returns>
        private bool BeginDrawFrame()
        {
            switch (_graphicsDevice.GraphicsDeviceStatus)
            {
                case GraphicsDeviceStatus.Normal:
                    // graphics device is okay
                    _graphicsDevice.ClearState();
                    if (_graphicsDevice.BackBuffer != null)
                    {
                        _graphicsDevice.SetRenderTargets(_graphicsDevice.DepthStencilBuffer, _graphicsDevice.BackBuffer);
                        _graphicsDevice.SetViewport(0, 0, _graphicsDevice.BackBuffer.Width, _graphicsDevice.BackBuffer.Height);
                    }

                    return true;

                default:
                    // graphics device needs to be recreated - give GPU driver some time to recover
                    Utilities.Sleep(TimeSpan.FromMilliseconds(20));

                    StopRendering();
                    Dispose();
                    CreateDeviceAndLoadContent();
                    StartRendering();

                    return false;
            }
        }

        /// <summary>
        /// Clears the render target.
        /// </summary>
        private void ClearRenderTarget()
        {
            _graphicsDevice.Clear(Color.Black);
        }

        /// <summary>
        /// Presents the drawn frame on control surface.
        /// </summary>
        private void EndDrawFrame()
        {
            try
            {
                _graphicsDevice.Present();
            }
            catch (SharpDXException ex)
            {
                Debug.WriteLine(ex);
                if (ex.ResultCode != SharpDX.DXGI.ResultCode.DeviceRemoved && ex.ResultCode != SharpDX.DXGI.ResultCode.DeviceReset)
                    throw;
            }
        }

        /// <summary>
        /// Loads scene-specific content.
        /// </summary>
        private void LoadContent()
        {
            // reusable structure representing a text font with size and style
            _textFormat = _disposeCollector.Collect(new DW.TextFormat(_dwFactory, "Consolas", SharpDX.DirectWrite.FontWeight.Normal, SharpDX.DirectWrite.FontStyle.Normal, 16f));

            // reusable brush structure
            _textBrush = _disposeCollector.Collect(new D2D.SolidColorBrush(_d2dDeviceContext, Color.LightGreen));

            // prebaked text - useful for constant labels as it greatly improves performance
            _textLayout = _disposeCollector.Collect(new DW.TextLayout(_dwFactory, "SharpDX Console Test.", _textFormat, 100f, 100f));

        }

        /// <summary>
        /// Draws scene-specific content.
        /// </summary>
        private void DrawContent()
        {
            bool beginDrawCalled = false;
            try
            {
                // begin a draw batch
                _d2dDeviceContext.BeginDraw();
                beginDrawCalled = true;

                if (m_console.CellHeight > 0 && m_console.CellWidth > 0)
                {
                    for (int x = 0; x < 80; x++)
                    {
                        for (int y = 0; y < 25; y++)
                        {
                            if (m_console.IsDisplayCellInvalid(x, y) || m_console.GetCharAtDisplayPos(x, y) != ' ')
                            {
                                char c = m_console.GetCharAtDisplayPos(x, y);
                                _textLayout = new DW.TextLayout(_dwFactory, c.ToString(), _textFormat, m_console.CellWidth, m_console.CellHeight);
                                //                            _textLayout = _disposeCollector.Collect(new DW.TextLayout(_dwFactory, c.ToString() , _textFormat, m_console.CellWidth, m_console.CellHeight));
                                _d2dDeviceContext.DrawTextLayout(new Vector2(x * m_console.CellWidth, y * m_console.CellHeight), _textLayout, _textBrush);
                                _textLayout.Dispose();
                            }
                        }
                    }
                }

                if (m_console.bCursorVisible)
                {
                    _d2dDeviceContext.FillRectangle(new RectangleF(m_console.iCursorX * m_console.CellWidth, m_console.iCursorY * m_console.CellHeight, m_console.CellWidth, (float)m_console.CellHeight * (float).6), _textBrush);
                }
            }
            finally
            {
                // end a draw batch
                if (beginDrawCalled)
                    _d2dDeviceContext.EndDraw();

            }
        }

        /// <summary>
        /// Handles the <see cref="SwapChainPanel.Loaded"/> event and prepares the control surface for rendering.
        /// </summary>
        /// <remarks>On first call - intializes surface-independent resources and loads content.</remarks>
        /// <param name="sender">Ignored.</param>
        /// <param name="e">Ignored.</param>
        private void HandleLoaded(object sender, RoutedEventArgs e)
        {
            // create the device only on first load after that it can be reused
            if (_graphicsDevice == null)
                CreateDeviceAndLoadContent();

            StartRendering();
        }

        /// <summary>
        /// Handles the <see cref="SwapChainPanel.Unloaded"/> evenet and stops rendering on the control surface.
        /// </summary>
        /// <remarks>Does not unload content nor disposes surface-independent resources.</remarks>
        /// <param name="sender">Ignored.</param>
        /// <param name="e">Ignored.</param>
        private void HandleUnloaded(object sender, RoutedEventArgs e)
        {
            StopRendering();
        }

        /// <summary>
        /// Handles the <see cref="SwapChainPanel.SizeChanged"/> event and resizes the surface-dependent resources accordingly.
        /// </summary>
        /// <param name="sender">Ignored.</param>
        /// <param name="e">The event arguments containing the new control size.</param>
        private void HandleSizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (e == null) throw new ArgumentNullException("e");

            ResetSize(e.NewSize);
        }

        /// <summary>
        /// Handles the <see cref="CompositionTarget.Rendering"/> event and redraws the control surface if necessary.
        /// </summary>
        /// <param name="sender">Ignored.</param>
        /// <param name="e">Ignored.</param>
        private void HandleRendering(object sender, object e)
        {
            PerformRendering();
        }

        async void CoreWindow_KeyUp(Windows.UI.Core.CoreWindow sender, Windows.UI.Core.KeyEventArgs args)
        {
            char res = (char)args.VirtualKey;

            if (args.VirtualKey == Windows.System.VirtualKey.Control)
            {
                bIsControl = false;
            }

            if (args.VirtualKey == Windows.System.VirtualKey.Shift)
            {
                bIsShifted = false;
            }

            // 188 ',' - 190 '.'

            if (res == 189)
            {
                if (!bIsShifted)
                    res = '-';
                else
                    res = '_';
            }

            if (res == 191)
            {
                if (!bIsShifted)
                    res = '/';
                else
                    res = '?';
            }

            if (res == 220)
            {
                if (!bIsShifted)
                    res = '\\';
                else
                    res = '|';
            }

            if (res == 188)
            {
                if (!bIsShifted)
                    res = ',';
                else
                    res = '<';
            }

            if (res == 190)
            {
                if (!bIsShifted)
                    res = '.';
                else
                    res = '>';
            }

            if (res >= '0' && res <= '9')
            {
                if (bIsShifted)
                {
                    res = (char)((int)res - 14);
                }
            }

            if (res >= 'A' && res <= 'Z')
            {
                if (bIsControl && res == 'C')
                {
                    res = (char)(int)3; // CTRL+C (Break).
                }
                else
                {
                    if (!bIsShifted)
                    {
                        res = (char)((int)res + 32);
                    }
                }
            }

            if (res >= ' ' && res <= 0x7f)
                m_console.WriteChar(res);

            if (args.VirtualKey == Windows.System.VirtualKey.Back)
            {
                m_console.WriteChar((char)args.VirtualKey);
            }

            if (args.VirtualKey == Windows.System.VirtualKey.Tab)
            {
                m_console.WriteChar((char)args.VirtualKey);
            }

            // NOTE: need to send each key to the telnet server
            // so things like ^C work to stop a long directory listing
            // or "TAB" works to auto-complete a line (dir a"TAB")

            if (args.VirtualKey == Windows.System.VirtualKey.Enter)
            {
                string sLine = m_console.CurrentLine;
                m_console.NewLine();
                if (!bTelnetConnected)
                {
                    if (sLine != string.Empty)
                    {
                        if (sLine.Length > 0)
                        {
                            // strip off the "Connect to: "
                            int iPos = sLine.LastIndexOf(' ');
                            string sConnect = string.Empty;
                            if (-1 != iPos)
                            {
                                sConnect = sLine.Substring(iPos + 1);
                            }

                            if (string.Empty != sConnect)
                            {
                                bTelnetConnected = true;
                                try
                                {
                                    await telnetClient.Connect(sConnect);
                                }
                                catch (Exception e)
                                {
                                    bTelnetConnected = false;
                                }
                                if (bTelnetConnected)
                                {
                                    telnetClient.Listen(WriteToConsoleOut);
                                    m_console.ClearDisplay();
                                }
                                else
                                {
                                    m_console.Write("Cannot connect to " + sConnect);
                                    m_console.NewLine();
                                    m_console.Write(sConnectString);
                                }
                            }
                            else
                            {   // You hit enter on an empty "Connect to: " prompt.
                                m_console.Write(sConnectString);
                            }
                        }
                    }
                }
                else
                {
                    if (sLine == string.Empty)
                        sLine = "";

                    try
                    {
                        await telnetClient.Write(sLine + "\r\n");
                    }
                    catch (Exception e)
                    {
                        // if we get here then the connection has been dropped.
                        bTelnetConnected = false;
                        telnetClient.Close();
                        m_console.WriteLine();
                        m_console.WriteLine("Connection lost");
                        m_console.Write(sConnectString);
                    }
                }

            }
        }

        private void WriteToConsoleOut(string str)
        {
            foreach (char c in str)
                m_console.ParseTelnetChar(c);
        }

        void CoreWindow_KeyDown(Windows.UI.Core.CoreWindow sender, Windows.UI.Core.KeyEventArgs args)
        {
            if (args.VirtualKey == Windows.System.VirtualKey.Control)
            {
                bIsControl = true;
            }

            if (args.VirtualKey == Windows.System.VirtualKey.Shift)
            {
                bIsShifted = true;
            }
        }

    }
}

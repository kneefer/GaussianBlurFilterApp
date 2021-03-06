using System;
using System.IO;
using System.Windows;
using System.Windows.Media;
using Gauss.GUI.Core;
using Gauss.GUI.Infrastructure;
using Gauss.GUI.Models;

namespace Gauss.GUI.ViewModels
{
    public sealed partial class MainWindowViewModel : ViewModelBase
    {
        private GaussImageManager ImageManager { get; set; }
        private string _mainPanelImagePath;
        private readonly GeneratorStopwatch _computationStopwatch;

        #region Properties

        public string InformationText
        {
            get { return _informationText; }
            set
            {
                if (value == _informationText) return;
                _informationText = value;
                OnPropertyChanged();
            }
        }
        private string _informationText;

        public int GaussMaskSize
        {
            get { return _gaussMaskSize; }
            set
            {
                if (Equals(value, _gaussMaskSize)) return;
                _gaussMaskSize = value;
                OnPropertyChanged();
            }
        }
        private int _gaussMaskSize;

        public byte[] MainPanelImage
        {
            get { return _mainPanelImage; }
            private set
            {
                if (Equals(value, _mainPanelImage)) return;
                _mainPanelImage = value;
                OnPropertyChanged();
            }
        }
        private byte[] _mainPanelImage;

        public SolidColorBrush ImageDropContainerBackground
        {
            get { return _imageDropContainerBackground; }
            private set
            {
                if (!Equals(value, _imageDropContainerBackground))
                {
                    _imageDropContainerBackground = value;
                    OnPropertyChanged();
                }
            }
        }
        private SolidColorBrush _imageDropContainerBackground;

        public string MainPanelDescription
        {
            get { return _mainPanelDescription; }
            set
            {
                if (value != _mainPanelDescription)
                {
                    _mainPanelDescription = value;
                    OnPropertyChanged();
                }
            }
        }
        private string _mainPanelDescription;

        public int NumberOfThreads
        {
            get { return _numberOfThreads; }
            set
            {
                if (value.Equals(_numberOfThreads)) return;
                _numberOfThreads = value;
                OnPropertyChanged();
            }
        }
        private int _numberOfThreads;

        public int BlurIterations
        {
            get { return _blurIterations; }
            set
            {
                if (value == _blurIterations) return;
                _blurIterations = value;
                OnPropertyChanged();
            }
        }
        private int _blurIterations;

        public GeneratingLibrary GeneratingLibrary
        {
            get { return _generatingLibrary; }
            set
            {
                if (value == _generatingLibrary) return;
                _generatingLibrary = value;
                OnPropertyChanged();
            }
        }
        private GeneratingLibrary _generatingLibrary;

        public ProgramState ProgramState
        {
            get { return _programState; }
            set
            {
                if (Equals(value, _programState)) return;
                _programState = value;
                OnPropertyChanged();
            }
        }
        private ProgramState _programState;

        public TimeSpan ComputationTime
        {
            get { return _computationTime; }
            set
            {
                if (value.Equals(_computationTime)) return;
                _computationTime = value;
                OnPropertyChanged();
            }
        }
        private TimeSpan _computationTime;

        #endregion

        #region Constructor

        public MainWindowViewModel()
        {
            InitializeCommands();
            InitializeProperties();

            _computationStopwatch = new GeneratorStopwatch(TimeSpan.FromMilliseconds(10));
            _computationStopwatch.Updated += updatedTime => { ComputationTime = updatedTime; };
        }

        #endregion

        private void InitializeProperties()
        {
            NumberOfThreads = 2;
            BlurIterations = 1;
            GeneratingLibrary = GeneratingLibrary.ASM;
            GaussMaskSize = 3;

            SetDropImageZoneState(DropImagesZoneState.Idle);
            InformationText = "Computing image...";
        }

        private void SetDropImageZoneState(DropImagesZoneState imagesZoneState)
        {
            Color colorToSet;
            string textToSet;

            switch (imagesZoneState)
            {
                case DropImagesZoneState.Idle:
                    colorToSet = Colors.WhiteSmoke;
                    textToSet = "Drop your BMP files here!";
                    break;
                case DropImagesZoneState.Valid:
                    colorToSet = Colors.DarkSeaGreen;
                    textToSet = "Now drop the files here!";
                    break;
                case DropImagesZoneState.Invalid:
                    colorToSet = Colors.PaleVioletRed;
                    textToSet = "Only BMP files are acceptable!";
                    break;
                case DropImagesZoneState.Dropped:
                    colorToSet = Colors.WhiteSmoke;
                    textToSet = String.Empty;
                    break;
                default:
                    throw new NotImplementedException();
            }

            ImageDropContainerBackground = new SolidColorBrush(colorToSet);
            MainPanelDescription = textToSet;
        }

        private void SaveShowingImage(string path)
        {
            try
            {
                File.WriteAllBytes(path, MainPanelImage);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        #region GUI Events

        public void OnImageDragOver(bool areImagesValid)
        {
            SetDropImageZoneState(areImagesValid ? DropImagesZoneState.Valid : DropImagesZoneState.Invalid);
        }

        public void OnImageDragLeave()
        {
            SetDropImageZoneState(DropImagesZoneState.Idle);
        }

        public void OnImageDrop(string droppedFile)
        {
            SetDropImageZoneState(DropImagesZoneState.Dropped);

            try
            {
                ImageManager = new GaussImageManager(droppedFile);

                MainPanelImage = File.ReadAllBytes(droppedFile);
                _mainPanelImagePath = droppedFile;

                ProgramState = ProgramState.ImageLoaded;
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
                SetDropImageZoneState(DropImagesZoneState.Idle);
            }
        }

        #endregion
    }
}
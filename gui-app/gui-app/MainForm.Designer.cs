namespace GuiApp
{
    partial class MainForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            loggerBox = new TextBox();
            viewport = new PictureBox();
            executeButton = new Button();
            sceneList = new ComboBox();
            sceneList_label = new Label();
            inputSPP_label = new Label();
            inputSPP = new NumericUpDown();
            inputPathLen_label = new Label();
            inputPathLen = new NumericUpDown();
            topLevelSplit = new SplitContainer();
            splitContainer1 = new SplitContainer();
            ((System.ComponentModel.ISupportInitialize)viewport).BeginInit();
            ((System.ComponentModel.ISupportInitialize)inputSPP).BeginInit();
            ((System.ComponentModel.ISupportInitialize)inputPathLen).BeginInit();
            ((System.ComponentModel.ISupportInitialize)topLevelSplit).BeginInit();
            topLevelSplit.Panel1.SuspendLayout();
            topLevelSplit.Panel2.SuspendLayout();
            topLevelSplit.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
            splitContainer1.Panel1.SuspendLayout();
            splitContainer1.Panel2.SuspendLayout();
            splitContainer1.SuspendLayout();
            SuspendLayout();
            // 
            // loggerBox
            // 
            loggerBox.Dock = DockStyle.Fill;
            loggerBox.Font = new Font("Courier New", 9.75F, FontStyle.Regular, GraphicsUnit.Point);
            loggerBox.Location = new Point(0, 0);
            loggerBox.Multiline = true;
            loggerBox.Name = "loggerBox";
            loggerBox.ReadOnly = true;
            loggerBox.ScrollBars = ScrollBars.Both;
            loggerBox.Size = new Size(365, 510);
            loggerBox.TabIndex = 0;
            loggerBox.WordWrap = false;
            // 
            // viewport
            // 
            viewport.BackColor = Color.Black;
            viewport.Dock = DockStyle.Fill;
            viewport.Location = new Point(8, 8);
            viewport.Name = "viewport";
            viewport.Size = new Size(617, 335);
            viewport.TabIndex = 1;
            viewport.TabStop = false;
            // 
            // executeButton
            // 
            executeButton.Font = new Font("Courier New", 9.75F, FontStyle.Regular, GraphicsUnit.Point);
            executeButton.Location = new Point(12, 100);
            executeButton.Name = "executeButton";
            executeButton.Size = new Size(550, 33);
            executeButton.TabIndex = 2;
            executeButton.Text = "Run raytracing";
            executeButton.UseVisualStyleBackColor = true;
            executeButton.Click += ExecuteButton_Click;
            // 
            // sceneList
            // 
            sceneList.FormattingEnabled = true;
            sceneList.Location = new Point(78, 10);
            sceneList.Name = "sceneList";
            sceneList.Size = new Size(121, 25);
            sceneList.TabIndex = 3;
            // 
            // sceneList_label
            // 
            sceneList_label.AutoSize = true;
            sceneList_label.Font = new Font("Courier New", 9.75F, FontStyle.Regular, GraphicsUnit.Point);
            sceneList_label.Location = new Point(12, 10);
            sceneList_label.Name = "sceneList_label";
            sceneList_label.Size = new Size(47, 16);
            sceneList_label.TabIndex = 4;
            sceneList_label.Text = "Scene";
            // 
            // inputSPP_label
            // 
            inputSPP_label.AutoSize = true;
            inputSPP_label.Font = new Font("Courier New", 9.75F, FontStyle.Regular, GraphicsUnit.Point);
            inputSPP_label.Location = new Point(247, 10);
            inputSPP_label.Name = "inputSPP_label";
            inputSPP_label.Size = new Size(143, 16);
            inputSPP_label.TabIndex = 5;
            inputSPP_label.Text = "Samples per pixel";
            // 
            // inputSPP
            // 
            inputSPP.Location = new Point(413, 10);
            inputSPP.Maximum = new decimal(new int[] { 4096, 0, 0, 0 });
            inputSPP.Minimum = new decimal(new int[] { 1, 0, 0, 0 });
            inputSPP.Name = "inputSPP";
            inputSPP.Size = new Size(120, 25);
            inputSPP.TabIndex = 6;
            inputSPP.Value = new decimal(new int[] { 10, 0, 0, 0 });
            // 
            // inputPathLen_label
            // 
            inputPathLen_label.AutoSize = true;
            inputPathLen_label.Font = new Font("Courier New", 9.75F, FontStyle.Regular, GraphicsUnit.Point);
            inputPathLen_label.Location = new Point(249, 50);
            inputPathLen_label.Name = "inputPathLen_label";
            inputPathLen_label.Size = new Size(127, 16);
            inputPathLen_label.TabIndex = 7;
            inputPathLen_label.Text = "Max path length";
            // 
            // inputPathLen
            // 
            inputPathLen.Location = new Point(413, 50);
            inputPathLen.Maximum = new decimal(new int[] { 1024, 0, 0, 0 });
            inputPathLen.Minimum = new decimal(new int[] { 1, 0, 0, 0 });
            inputPathLen.Name = "inputPathLen";
            inputPathLen.Size = new Size(120, 25);
            inputPathLen.TabIndex = 8;
            inputPathLen.Value = new decimal(new int[] { 5, 0, 0, 0 });
            // 
            // topLevelSplit
            // 
            topLevelSplit.Dock = DockStyle.Fill;
            topLevelSplit.IsSplitterFixed = true;
            topLevelSplit.Location = new Point(0, 0);
            topLevelSplit.Name = "topLevelSplit";
            // 
            // topLevelSplit.Panel1
            // 
            topLevelSplit.Panel1.Controls.Add(splitContainer1);
            // 
            // topLevelSplit.Panel2
            // 
            topLevelSplit.Panel2.Controls.Add(loggerBox);
            topLevelSplit.Size = new Size(1002, 510);
            topLevelSplit.SplitterDistance = 633;
            topLevelSplit.TabIndex = 1;
            // 
            // splitContainer1
            // 
            splitContainer1.Dock = DockStyle.Fill;
            splitContainer1.IsSplitterFixed = true;
            splitContainer1.Location = new Point(0, 0);
            splitContainer1.Name = "splitContainer1";
            splitContainer1.Orientation = Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            splitContainer1.Panel1.Controls.Add(viewport);
            splitContainer1.Panel1.Padding = new Padding(8);
            // 
            // splitContainer1.Panel2
            // 
            splitContainer1.Panel2.Controls.Add(sceneList_label);
            splitContainer1.Panel2.Controls.Add(sceneList);
            splitContainer1.Panel2.Controls.Add(inputSPP);
            splitContainer1.Panel2.Controls.Add(inputSPP_label);
            splitContainer1.Panel2.Controls.Add(inputPathLen_label);
            splitContainer1.Panel2.Controls.Add(inputPathLen);
            splitContainer1.Panel2.Controls.Add(executeButton);
            splitContainer1.Size = new Size(633, 510);
            splitContainer1.SplitterDistance = 351;
            splitContainer1.TabIndex = 2;
            // 
            // MainForm
            // 
            AutoScaleDimensions = new SizeF(7F, 17F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(1002, 510);
            Controls.Add(topLevelSplit);
            Font = new Font("Malgun Gothic", 9.75F, FontStyle.Regular, GraphicsUnit.Point);
            Name = "MainForm";
            Text = "Software Raytracer";
            Load += MainForm_Load;
            ((System.ComponentModel.ISupportInitialize)viewport).EndInit();
            ((System.ComponentModel.ISupportInitialize)inputSPP).EndInit();
            ((System.ComponentModel.ISupportInitialize)inputPathLen).EndInit();
            topLevelSplit.Panel1.ResumeLayout(false);
            topLevelSplit.Panel2.ResumeLayout(false);
            topLevelSplit.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)topLevelSplit).EndInit();
            topLevelSplit.ResumeLayout(false);
            splitContainer1.Panel1.ResumeLayout(false);
            splitContainer1.Panel2.ResumeLayout(false);
            splitContainer1.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
            splitContainer1.ResumeLayout(false);
            ResumeLayout(false);
        }

        #endregion

        private TextBox loggerBox;
        private PictureBox viewport;
        private Button executeButton;
        private ComboBox sceneList;
        private Label sceneList_label;
        private Label inputSPP_label;
        private NumericUpDown inputSPP;
        private Label inputPathLen_label;
        private NumericUpDown inputPathLen;
        private SplitContainer topLevelSplit;
        private SplitContainer splitContainer1;
    }
}
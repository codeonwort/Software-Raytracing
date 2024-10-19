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
            this.loggerBox = new System.Windows.Forms.TextBox();
            this.viewport = new System.Windows.Forms.PictureBox();
            this.executeButton = new System.Windows.Forms.Button();
            this.sceneList = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.inputSPP = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.inputPathLen = new System.Windows.Forms.NumericUpDown();
            ((System.ComponentModel.ISupportInitialize)(this.viewport)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.inputSPP)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.inputPathLen)).BeginInit();
            this.SuspendLayout();
            // 
            // loggerBox
            // 
            this.loggerBox.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.loggerBox.Location = new System.Drawing.Point(592, 12);
            this.loggerBox.Multiline = true;
            this.loggerBox.Name = "loggerBox";
            this.loggerBox.ReadOnly = true;
            this.loggerBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.loggerBox.Size = new System.Drawing.Size(398, 486);
            this.loggerBox.TabIndex = 0;
            this.loggerBox.WordWrap = false;
            // 
            // viewport
            // 
            this.viewport.BackColor = System.Drawing.Color.Black;
            this.viewport.Location = new System.Drawing.Point(12, 12);
            this.viewport.Name = "viewport";
            this.viewport.Size = new System.Drawing.Size(550, 346);
            this.viewport.TabIndex = 1;
            this.viewport.TabStop = false;
            // 
            // executeButton
            // 
            this.executeButton.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.executeButton.Location = new System.Drawing.Point(12, 465);
            this.executeButton.Name = "executeButton";
            this.executeButton.Size = new System.Drawing.Size(550, 33);
            this.executeButton.TabIndex = 2;
            this.executeButton.Text = "Run raytracing";
            this.executeButton.UseVisualStyleBackColor = true;
            this.executeButton.Click += new System.EventHandler(this.ExecuteButton_Click);
            // 
            // sceneList
            // 
            this.sceneList.FormattingEnabled = true;
            this.sceneList.Location = new System.Drawing.Point(78, 374);
            this.sceneList.Name = "sceneList";
            this.sceneList.Size = new System.Drawing.Size(121, 25);
            this.sceneList.TabIndex = 3;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.label1.Location = new System.Drawing.Point(12, 378);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(47, 16);
            this.label1.TabIndex = 4;
            this.label1.Text = "Scene";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.label2.Location = new System.Drawing.Point(247, 378);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(143, 16);
            this.label2.TabIndex = 5;
            this.label2.Text = "Samples per pixel";
            // 
            // inputSPP
            // 
            this.inputSPP.Location = new System.Drawing.Point(413, 374);
            this.inputSPP.Maximum = new decimal(new int[] {
            4096,
            0,
            0,
            0});
            this.inputSPP.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.inputSPP.Name = "inputSPP";
            this.inputSPP.Size = new System.Drawing.Size(120, 25);
            this.inputSPP.TabIndex = 6;
            this.inputSPP.Value = new decimal(new int[] {
            10,
            0,
            0,
            0});
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.label3.Location = new System.Drawing.Point(249, 415);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(127, 16);
            this.label3.TabIndex = 7;
            this.label3.Text = "Max path length";
            // 
            // inputPathLen
            // 
            this.inputPathLen.Location = new System.Drawing.Point(413, 412);
            this.inputPathLen.Maximum = new decimal(new int[] {
            1024,
            0,
            0,
            0});
            this.inputPathLen.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.inputPathLen.Name = "inputPathLen";
            this.inputPathLen.Size = new System.Drawing.Size(120, 25);
            this.inputPathLen.TabIndex = 8;
            this.inputPathLen.Value = new decimal(new int[] {
            5,
            0,
            0,
            0});
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 17F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1002, 510);
            this.Controls.Add(this.inputPathLen);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.inputSPP);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.sceneList);
            this.Controls.Add(this.executeButton);
            this.Controls.Add(this.viewport);
            this.Controls.Add(this.loggerBox);
            this.Font = new System.Drawing.Font("Malgun Gothic", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.Name = "MainForm";
            this.Text = "MainForm";
            this.Load += new System.EventHandler(this.MainForm_Load);
            ((System.ComponentModel.ISupportInitialize)(this.viewport)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.inputSPP)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.inputPathLen)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private TextBox loggerBox;
        private PictureBox viewport;
        private Button executeButton;
        private ComboBox sceneList;
        private Label label1;
        private Label label2;
        private NumericUpDown inputSPP;
        private Label label3;
        private NumericUpDown inputPathLen;
    }
}